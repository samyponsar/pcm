// author: fangjun kuang <csukuangfj at gmail dot com>
// date: Apr. 22, 2019

// refer to http://www.topherlee.com/software/pcm-tut-wavformat.html

#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <filesystem>
#include <sys/stat.h>

using namespace std;

const double pi = 3.14159265358979323846;

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
    uint16_t NumOfChan = 1;   // Number of channels 1=Mono 2=Stereo
    uint32_t SamplesPerSec = 44100;   // Sampling Frequency in Hz
    uint32_t bytesPerSec = 44100 * 2; // bytes per second
    uint16_t blockAlign = 2;          // 2=16-bit mono, 4=16-bit stereo
    uint16_t bitsPerSample = 16;      // Number of bits per sample
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
    cout << fsize << "\n";
    
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
	int length = (int) (seconds * 44100);
	vector<double> result;
	for (double x = 0; x < length; x ++) {
		double samplevalue = sin((x * 2 * pi * frequency) / 44100) * amplitude;
		result.push_back(samplevalue);
	}
	return result;
}


int main() {
    double seconds, frequency, amplitude;
    string filename;
    cout << "Duration? (in seconds) ";
    while(!(cin >> seconds) || seconds < 0) {
      cin.clear();
      cin.ignore(1000, '\n');
      cout << "Invalid input.\nDuration? (in seconds) ";
    }
    cout << "Frequency? (Hz) ";
    while(!(cin >> frequency) || frequency < 0) {
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
    cout << "File name? ";
    while(!(cin >> filename)) {
      cin.clear();
      cin.ignore(1000, '\n');
      cout << "Invalid input.\nFile name? ";
    }
    filename += ".wav";
    ofstream out("temp.bin", ios::binary);
    vector<double> data = sine(seconds, frequency, amplitude);
    for (double i: data) {
      int16_t j = (int16_t)(i*32767);  
      out.write(reinterpret_cast<char*>(&j), sizeof(int16_t));
    }
    out.close();
    
    makewav(filename);
    remove("temp.bin");
    return 0;
}
