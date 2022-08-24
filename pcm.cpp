// refer to http://www.topherlee.com/software/pcm-tut-wavformat.html

#define _USE_MATH_DEFINES

#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <limits>
#include <cassert>
#include <regex>

using namespace std;

const int CHANNELS = 1;
const int SAMPLERATE = 44100;
const int BITRATE = 16;

vector<double> audiodata; //raw PCM data

typedef struct WAV_HEADER {
    /* RIFF Chunk Descriptor */
    uint8_t RIFF[4] = { 'R', 'I', 'F', 'F' }; // RIFF Header Magic header
    uint32_t ChunkSize;                     // RIFF Chunk Size
    uint8_t WAVE[4] = { 'W', 'A', 'V', 'E' }; // WAVE Header
    /* "fmt" sub-chunk */
    uint8_t fmt[4] = { 'f', 'm', 't', ' ' }; // FMT header
    uint32_t Subchunk1Size = 16;           // Size of the fmt chunk
    uint16_t AudioFormat = 1; // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM
    // Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t NumOfChan = CHANNELS;   // Number of channels 1=Mono 2=Stereo
    uint32_t SamplesPerSec = SAMPLERATE;   // Sampling Frequency in Hz
    uint32_t bytesPerSec = SAMPLERATE * (BITRATE/8); // bytes per second
    uint16_t blockAlign = CHANNELS*(BITRATE/8);          // 2=16-bit mono, 4=16-bit stereo
    uint16_t bitsPerSample = BITRATE;      // Number of bits per sample
    /* "data" sub-chunk */
    uint8_t Subchunk2ID[4] = { 'd', 'a', 't', 'a' }; // "data"  string
    uint32_t Subchunk2Size;                        // Sampled data length
} wav_hdr;

int makewav(string filename) {
    
    uint32_t datasize = (uint32_t)(audiodata.size()*(BITRATE/8));
    wav_hdr wav;
    wav.ChunkSize = (datasize + sizeof(wav_hdr) - 8);
    wav.Subchunk2Size = datasize;

    ofstream out(filename, ios::binary);
    
    assert (sizeof(wav) == 44); //check header length
    out.write((char *)&wav, sizeof(wav)); //write header
    
    for (double i: audiodata) {
      assert (-1 <= i <= 1);
      int16_t j = (int16_t)(i*INT16_MAX); //map to 16bit range but preserve symmetry (min value = -32767)
      out.write((char *)&j, sizeof(int16_t)); //write data
    }
    if (out.fail()){
        cout << "File write error!\n";
    }
    out.close();
    return 0;
}

vector<double> sine(double seconds, double frequency, double amplitude) {
    int length = (int) (seconds * SAMPLERATE);
    double period = (1/frequency) * SAMPLERATE;
    vector<double> result;
    for (int x = 0; x < length; x ++) {
        double samplevalue = sin((x*2*M_PI)/period) * amplitude;
        result.push_back(samplevalue);
    }
    return result;
}

vector<double> square(double seconds, double frequency, double amplitude) {
    vector<double> result = sine(seconds, frequency, 1);
    for (double &i: result) { 
        i = ((i >= 0) - (i < 0)) * amplitude; //sign function, 0 becomes +1
    }
    return result;
}

vector<double> triangle(double seconds, double frequency, double amplitude) {
    int length = (int) (seconds * SAMPLERATE);
    double period = (1/frequency) * SAMPLERATE;
    vector<double> result;
    for (int x = 0; x < length; x ++) {
        double samplevalue = (4*amplitude/period)*abs(fmod((x+period-(period/4)),period)-(period/2))-amplitude;
        result.push_back(samplevalue);
    }
    return result;
}

vector<double> saw(double seconds, double frequency, double amplitude) {
    int length = (int) (seconds * SAMPLERATE);
    double period = (1/frequency) * SAMPLERATE;
    vector<double> result;
    for (int x = 0; x < length; x ++) {
        double samplevalue = (((fmod(x, period))/period)-0.5)*2*amplitude;
        result.push_back(samplevalue);
    }
    return result;
}

vector<double> sawr(double seconds, double frequency, double amplitude) {
    vector<double> result = saw(seconds, frequency, amplitude);
    for (double &i: result) {
        i = -i;
    }
    return result;     
}

int main() {
    int mode;
    double seconds, frequency, amplitude;
    string filename;
    cout << "Duration? (seconds) ";
    while(!(cin >> seconds) || seconds < 0 || seconds > 1000000) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid input.\nDuration? (seconds) ";
    }
    cout << "Frequency? (Hz) ";
    while(!(cin >> frequency) || frequency <= 0) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid input.\nFrequency? (Hz) ";
    }
    if (frequency > SAMPLERATE/2) {
        cout << "WARNING: Input too high, aliasing will occur.\n";
    }
    
    cout << "Amplitude? (0-1) ";
    while(!(cin >> amplitude) || amplitude < 0 || amplitude > 1) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid input.\nAmplitude? (0-1) ";
    }
    if (amplitude < 0.05) {
        cout << "WARNING: Amplitude very low and dangerously close to noise floor.\n";
    }

    cout << "Waveform mode? (1=sine | 2=triangle | 3=square | 4=saw | 5=reverse saw) ";
    while ( !(cin >> mode) || !(mode == 1 || mode == 2 || mode == 3 || mode == 4 || mode == 5) ) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid input.\nWaveform mode? (1=sine | 2=triangle | 3=square | 4=saw | 5=reverse saw) ";
    }
    switch (mode) {
        case 1:
            audiodata = sine(seconds, frequency, amplitude);
            break;
        case 2:
            audiodata = triangle(seconds, frequency, amplitude);
            break;
        case 3:
            audiodata = square(seconds, frequency, amplitude);
            break;
        case 4:
            audiodata = saw(seconds, frequency, amplitude);
            break;
        case 5:
            audiodata = sawr(seconds, frequency, amplitude);
            break;
    }
    
    cout << "File name? ";
    while (filename.length() == 0) {     
	getline(cin, filename); 
    }
    while (filename.length() > 100 || !(regex_match(filename, regex("[\\w\\-. ]+")))) {
        filename.clear();
        cout << "Invalid input.\nFile name? ";
        while (filename.length() == 0) {     
	    getline(cin, filename); 
        }
    }
    filename += ".wav";
    makewav(filename);
    
    cout << "Success! Press Enter to exit.";
    getchar();
    return 0;
}
