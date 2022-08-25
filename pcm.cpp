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
#include <algorithm>
#include <functional>

using namespace std;

const int CHANNELS = 1;
const int SAMPLERATE = 44100;
const int BITRATE = 16;

vector<double> audioData; //final raw PCM data

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

class Osc{

        int mode;
        double duration, frequency, amplitude;

    public:
        vector<double> oscData;

        void init();
        vector<double> sine();
        vector<double> triangle();
        vector<double> square();
        vector<double> saw();
        vector<double> sawr();
};

void Osc::init(){
    cout << "Osc mode? (1=sine | 2=triangle | 3=square | 4=saw | 5=reverse saw) ";
    while ( !(cin >> mode) || !(mode == 1 || mode == 2 || mode == 3 || mode == 4 || mode == 5) ) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid input.\nOsc mode? (1=sine | 2=triangle | 3=square | 4=saw | 5=reverse saw) ";
    }
    cout << "Duration? (seconds) ";
    while(!(cin >> duration) || duration < 0 || duration > 1000000) {
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
    cout << "Amplitude? (0-100, 1 = 0dBFS) ";
    while(!(cin >> amplitude) || amplitude < 0 || amplitude > 100) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid input.\nAmplitude? (0-100, 1 = 0dBFS) ";
    }
    if (amplitude < 0.05) {
        cout << "WARNING: Amplitude very low and dangerously close to noise floor.\n";
    }

    switch (mode) {
        case 1:
            oscData = sine();
            break;
        case 2:
            oscData = triangle();
            break;
        case 3:
            oscData = square();
            break;
        case 4:
            oscData = saw();
            break;
        case 5:
            oscData = sawr();
            break;
    }
}

vector<double> Osc::sine() {
    int length = (int) (duration * SAMPLERATE);
    double period = (1/frequency) * SAMPLERATE;
    vector<double> result;
    for (int x = 0; x < length; x ++) {
        double samplevalue = sin((x*2*M_PI)/period) * amplitude;
        result.push_back(samplevalue);
    }
    return result;
}

vector<double> Osc::triangle() {
    int length = (int) (duration * SAMPLERATE);
    double period = (1/frequency) * SAMPLERATE;
    vector<double> result;
    for (int x = 0; x < length; x ++) {
        double samplevalue = (4*amplitude/period)*abs(fmod((x+period-(period/4)),period)-(period/2))-amplitude;
        result.push_back(samplevalue);
    }
    return result;
}

vector<double> Osc::square() {
    int length = (int) (duration * SAMPLERATE);
    double period = (1/frequency) * SAMPLERATE;
    vector<double> result;
    for (int x = 0; x < length; x ++) {
        double samplevalue = sin((x*2*M_PI)/period);
        samplevalue = ((samplevalue >= 0) - (samplevalue < 0)) * amplitude;
        result.push_back(samplevalue);
    }
    return result;
}

vector<double> Osc::saw() {
    int length = (int) (duration * SAMPLERATE);
    double period = (1/frequency) * SAMPLERATE;
    vector<double> result;
    for (int x = 0; x < length; x ++) {
        double samplevalue = (((fmod(x, period))/period)-0.5)*2*amplitude;
        result.push_back(samplevalue);
    }
    return result;
}

vector<double> Osc::sawr() {
    int length = (int) (duration * SAMPLERATE);
    double period = (1/frequency) * SAMPLERATE;
    vector<double> result;
    for (int x = 0; x < length; x ++) {
        double samplevalue = -(((fmod(x, period))/period)-0.5)*2*amplitude;
        result.push_back(samplevalue);
    }
    return result;
}

int makewav(string filename) {
    
    uint32_t datasize = (uint32_t)(audioData.size()*(BITRATE/8));
    wav_hdr wav;
    wav.ChunkSize = (datasize + sizeof(wav_hdr) - 8);
    wav.Subchunk2Size = datasize;

    ofstream out(filename, ios::binary);
    
    assert (sizeof(wav) == 44); //check header length
    out.write((char *)&wav, sizeof(wav)); //write header
    
    for (double &i: audioData) {
      int16_t j = (int16_t)(clamp<double>(i,-1,1)*INT16_MAX); //map to 16bit range & preserve symmetry (min = -32767)
      out.write((char *)&j, sizeof(int16_t)); //write data
    }
    if (out.fail()){
        cout << "File write error!\n";
    }
    out.close();
    return 0;
}

int main() {
    int oscCount = 0;
    char another = 'y';
    Osc osc;
    while (another == 'y') {
        oscCount ++;
        cout << "Oscillator #" << oscCount << ':' << endl;
        osc.init();
        audioData.resize(max(audioData.size(), osc.oscData.size()));
        //add osc data to the final audio data
        transform(osc.oscData.begin(), osc.oscData.end(), audioData.begin(), audioData.begin(), plus<double>());
        cout << "Add another oscillator? (y/n) ";
        while(!(cin >> another) || !(another == 'y' || another == 'n')) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input.\nAdd another oscillator? (y/n) ";
        }
}
    string filename;
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
    
//    cout << "Success! Press Enter to exit.";
//    getchar();
    return 0;
}
