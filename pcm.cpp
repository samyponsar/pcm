// refer to http://www.topherlee.com/software/pcm-tut-wavformat.html

#define _USE_MATH_DEFINES

#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <limits>
#include <cassert>


using namespace std;

const int CHANNELS = 1;
const int SAMPLERATE = 44100;
const int BITRATE = 16;

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
    static_assert(sizeof(wav_hdr) == 44, "");
    
    ifstream in("temp.bin", ios::binary);
    in.seekg(0, in.end);
    uint32_t fsize = in.tellg();
    in.seekg(0, in.beg);
    
    wav_hdr wav;
    wav.ChunkSize = (fsize + sizeof(wav_hdr) - 8);
    wav.Subchunk2Size = fsize;

    ofstream out(filename, ios::binary);
    out.write(reinterpret_cast<const char*>(&wav), sizeof(wav));

    int8_t d;
    for (uint32_t i = 0; i < fsize; ++i) {
        in.read(reinterpret_cast<char*>(&d), sizeof(int8_t));
        out.write(reinterpret_cast<char*>(&d), sizeof(int8_t));
    }
    in.close();
    out.close();
    return 0;
}



vector<double> sine(double seconds, double frequency, double amplitude) {
    int length = (int) (seconds * SAMPLERATE);
    double period = (1/frequency) * SAMPLERATE;
    vector<double> result;
    for (int x = 0; x < length; x ++) {
        assert ((x * 2 * M_PI * frequency) < numeric_limits<double>::infinity());
        assert ((x * 2 * M_PI * frequency) >= 0);
        double samplevalue = sin((x*2*M_PI)/period) * amplitude;
        result.push_back(samplevalue);
    }
    return result;
}

vector<double> square(double seconds, double frequency, double amplitude) {
    vector<double> result = sine(seconds, frequency, 1);
    for (int x = 0; x < result.size(); x++) {
        if (result[x] >= 0) {
            result[x] = amplitude;
        }
        else {
            result[x] = -amplitude;
        }
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
    for (int x = 0; x < result.size(); x++) {
        result[x] = -result[x];
    }
    return result;     
}

int main() {
    int mode;
    double seconds, frequency, amplitude;
    string filename;
    cout << "Duration? (in seconds) ";
    while(!(cin >> seconds) || seconds < 0) {
      cin.clear();
      cin.ignore(1000, '\n');
      cout << "Invalid input.\nDuration? (in seconds) ";
    }
    cout << "Frequency? (Hz) ";
    while(!(cin >> frequency) || frequency <= 0) {
      cin.clear();
      cin.ignore(1000, '\n');
      cout << "Invalid input.\nFrequency? (Hz) ";
    }
    cout << "Amplitude? (0-1) ";
    while(!(cin >> amplitude) || amplitude < 0 || amplitude > 1) {
      cin.clear();
      cin.ignore(1000, '\n');
      cout << "Invalid input.\nAmplitude? (0-1) ";
    }
    ofstream out("temp.bin", ios::binary);

    cout << "Waveform mode? (1=sine | 2=triangle | 3=square | 4=saw | 5=reverse saw) ";
    while ( !(cin >> mode) || !(mode == 1 || mode == 2 || mode == 3 || mode == 4 || mode == 5) ) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid input.\nWaveform mode? (1=sine | 2=triangle | 3=square | 4=saw | 5=reverse saw) ";
    }
    vector<double> data;
    switch (mode) {
        case 1:
            data = sine(seconds, frequency, amplitude);
            break;
        case 2:
            data = triangle(seconds, frequency, amplitude);
            break;
        case 3:
            data = square(seconds, frequency, amplitude);
            break;
        case 4:
            data = saw(seconds, frequency, amplitude);
            break;
        case 5:
            data = sawr(seconds, frequency, amplitude);
            break;
    }

    for (double i: data) {
      int16_t j = (int16_t)(i*32767);  
      out.write(reinterpret_cast<char*>(&j), sizeof(int16_t));
    }
    out.close();

    cout << "File name? ";
    while (!(cin >> filename)) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid input.\nFile name? ";
    }
    filename += ".wav";
    makewav(filename);
    remove("temp.bin");
    return 0;
}
