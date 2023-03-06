#include "StandardFFTProcessor.h"

//#pragma once
#include "utilities.h"

StandardFFTProcessor::StandardFFTProcessor(int size, int hopSize, int offset, int sRate) :
    m_sRate(sRate), m_fftSize(size), m_halfSize(size / 2), m_invSize(1.f/float(size)),
    m_hopsize(hopSize), m_offset(offset), m_hann(size), m_input(size),
    m_cpxInput(size), m_fftOut(size), m_polarIn(m_halfSize),m_polarOut(m_halfSize),
    m_ifftin(size), m_cpxOutput(size), m_output(size),
    fft(new kissfft<FftDecimal>(m_fftSize, false)), ifft(new kissfft<FftDecimal> (m_fftSize, true))
{
    fillHann(m_hann, m_fftSize);
}


void StandardFFTProcessor::process(const FftDecimal* input, FftDecimal* output, int blockSize){

    fill_in_passOut(input, output, blockSize);
    m_offset += blockSize;

    // when the input buffer reaches the fft block size we can fft
    if( m_offset >= m_fftSize ){
        doHann(m_input);
        // here is a good location for a zero padding function
       // zeroPadding(m_input, int(m_fftSize*0.8));
       //m_input.resize(m_fftSize, 0.f);
        float2Cpx(m_input, m_cpxInput);

        fft->transform(&m_cpxInput[0], &m_fftOut[0]);

        normalise(m_fftOut);
        utilities::car2Pol(m_fftOut, m_polarOut, m_halfSize);

        spectral_process(m_polarOut, m_polarIn, m_halfSize);

        utilities::pol2Car(m_polarIn, m_ifftin, m_halfSize);
        m_ifftin[0] = 0.f;

        ifft->transform(&m_ifftin[0], &m_cpxOutput[0]);
                
        cpx2Float(m_cpxOutput, m_output);
        doHann(m_output);
        m_offset = 0; //reset count
    }
}

bool StandardFFTProcessor::setFFTSize(int newSize){
    m_fftSize = newSize;
    m_halfSize = newSize/2;
    m_invSize = 1.f/(float)newSize;

    Cpx emptyCpx(0, 0);
    m_input.resize(newSize, 0.f);
    m_cpxInput.resize(newSize, emptyCpx);
    m_output.resize(newSize, 0.f);
    m_fftOut.resize(newSize, emptyCpx);
    m_hann.resize(newSize, 0.f);

    fillHann(m_hann, newSize);

    Polar<FftDecimal> emptyPolar(0.f, 0.f);
    m_polarOut.resize(newSize, emptyPolar);
    m_polarIn.resize(newSize, emptyPolar);
    m_ifftin.resize(newSize, emptyCpx);
    m_cpxOutput.resize(newSize, emptyCpx);
    if(newFFT(newSize) == 1) return false;

    return true;
}


void StandardFFTProcessor::fillHann(std::vector<FftDecimal>& table, int size){
    float w;
    float invert_Cos;
    float max = size - 1;
    for(int n = 0; n < size; ++n){
        //hann window -- inverted cosine
        w = (float)TWOPI * ( float(n)/max );
        invert_Cos = 1.f - cos(w);
        //normalise to 0 to 1
        table[n] = 0.5f * invert_Cos;
    }
}

void StandardFFTProcessor::fillBlackmanWindow(std::vector<FftDecimal>& table, int size) {
    float max = size - 1;
    for(int n=0; n < size; ++n) {
        float w1 = (float)TWOPI * (float(n)/max);
        float w2 = (float)TWOPI*2.0 * (float(n)/max);
        float c1 = 0.5 * cos(w1);
        float c2 = 0.08 * cos(w2);
        table[n] = 0.42 - (c1 + c2);
    }
}

void StandardFFTProcessor::fillHamming(std::vector<FftDecimal>& table, int size) {
    float max = size + 1.0;
    for(int n=0; n < size; ++n) {
        float w = (float)TWOPI * (float(n) / max);
        table[n] = 0.53836 - 0.46164 * cos(w);
    }
}

void StandardFFTProcessor::fillBlackmanHarris(std::vector<FftDecimal>& table, int size) {
   // 0.35875-0.48829*cos(2*M_PI*i/(nsamples-1.0))+0.14128*cos(4*M_PI*i/(nsamples-1.0))-0.01168*cos(6*M_PI*i/(nsamples-1.0));
    float max = size - 1;
    for(int n=0; n<size; ++n) {
        float increment = (float(n)/max);
        float w1 = (float)TWOPI * increment;
        float w2 = (float)TWOPI*2.0 * increment;
        float w3 = (float)TWOPI*6.0 * increment;
        
        float c1 = 0.48829 * cos(w1);
        float c2 = 0.14128 * cos(w2);
        float c3 = 0.01168 * cos(w3);
        
        table[n] = 0.35875 - c1 + c2 - c3;
    }
}

int StandardFFTProcessor::newFFT(int newSize){
    if(fft != nullptr) {
        delete fft;
        fft = nullptr;
    }
    
    fft = new kissfft<FftDecimal>(newSize, false);

// TODO: for the time stretching plugin we only will delete the inverse fft array when the stretch amount changes
//    if(ifft != nullptr) {
//       delete ifft;
//       ifft = nullptr;
//    }
    //ifft = new kissfft<float>(524288, true);

    if(ifft != nullptr) {
       delete ifft;
       ifft = nullptr;
    }
    ifft = new kissfft<FftDecimal>(newSize, true);
    
    if(fft == nullptr || ifft == nullptr) return 1;
    return 0;
}

void StandardFFTProcessor::fill_in_passOut(const FftDecimal* audioInput, FftDecimal* processOutput, int blockSize){

	 // this will write the audio input to the internal buffer of the process.
    FftDecimal* pWriteToProcess = (&m_input[0]) + m_offset;
    FftDecimal* pWriteToOutput = (&m_output[0]) + m_offset;

    for(int i=0; i<blockSize; ++i){
        *pWriteToProcess = audioInput[i];
        processOutput[i] += *pWriteToOutput;
        ++pWriteToProcess;
        ++pWriteToOutput;
    }
}


void StandardFFTProcessor::doHann(std::vector<FftDecimal>& inOut){
  for(int i=0; i<m_fftSize; ++i){
    inOut[i] *= m_hann[i];
  }
}

void StandardFFTProcessor::float2Cpx(const std::vector<FftDecimal>& inFloat, std::vector<Cpx>& outCpx){
  for(int i=0; i<m_fftSize; ++i){
	outCpx[i].real(inFloat[i]);    
	outCpx[i].imag(0.f);
  }
}

void StandardFFTProcessor::cpx2Float(const std::vector<Cpx>& inCpx, std::vector<FftDecimal>& outFloat){
  for(int i=0; i<m_fftSize; ++i){
    outFloat[i] = inCpx[i].real();
  }
}

void StandardFFTProcessor::normalise(std::vector<Cpx>& cmpxInOut){
    
    float scale = m_invSize * 2.0;
	for(int i=0; i<m_halfSize; ++i){	  		
        float normalisedRealValue = cmpxInOut[i].real() * scale;
		cmpxInOut[i].real(normalisedRealValue);

        float normalisedImaginaryValue = cmpxInOut[i].imag() * scale;
		cmpxInOut[i].imag(normalisedImaginaryValue);
	}
    
}
