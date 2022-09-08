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

static const int INT24_MAX = 8388607;

class Audio {
public:
	static unsigned int CHANNELS;
	static unsigned int SAMPLERATE;
	static unsigned int BITRATE;
	vector<double> pcmData;
	void set();
	void sum(vector<double>);
}audio;

unsigned int Audio::CHANNELS = 2;
unsigned int Audio::SAMPLERATE = 44100;
unsigned int Audio::BITRATE = 24;

void Audio::set() {

	cout << "Channels? (1=mono | 2=stereo) ";
	while (!(cin >> CHANNELS) || !(CHANNELS == 1 || CHANNELS == 2)) {
		cin.clear();
		cin.ignore(1000, '\n');
		cout << "Invalid input.\nChannels? (1=mono | 2=stereo) ";
	}

	cout << "Sample rate? (8000-352800 Hz) ";
	while (!(cin >> SAMPLERATE) || SAMPLERATE < 8000 || SAMPLERATE > 352800) {
		cin.clear();
		cin.ignore(1000, '\n');
		cout << "Invalid input.\nSample rate? (8000-352800 Hz) ";
	}

	cout << "Bit rate? (16 | 24) ";
	while (!(cin >> BITRATE) || !(BITRATE == 16 || BITRATE == 24)) {
		cin.clear();
		cin.ignore(1000, '\n');
		cout << "Invalid input.\nBit rate? (16 | 24) ";
	}
}

void Audio::sum(vector<double> other) {
	pcmData.resize(max(other.size(), pcmData.size()));
	transform(other.begin(), other.end(), pcmData.begin(), pcmData.begin(), plus<double>());
}

class Final : public Audio {
public:
	void makewav();
}final;

void Final::makewav() {
	typedef struct WAV_HEADER {
		/* RIFF Chunk Descriptor */
		uint8_t RIFF[4] = { 'R', 'I', 'F', 'F' }; // RIFF Header Magic header
		uint32_t ChunkSize = 0;                     // RIFF Chunk Size
		uint8_t WAVE[4] = { 'W', 'A', 'V', 'E' }; // WAVE Header
		/* "fmt" sub-chunk */
		uint8_t fmt[4] = { 'f', 'm', 't', ' ' }; // FMT header
		uint32_t Subchunk1Size = 16;           // Size of the fmt chunk
		uint16_t AudioFormat = 1; // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM
		// Mu-Law, 258=IBM A-Law, 259=ADPCM
		uint16_t NumOfChan = (uint16_t) CHANNELS;   // Number of channels 1=Mono 2=Stereo
		uint32_t SamplesPerSec = (uint32_t) SAMPLERATE;   // Sampling Frequency in Hz
		uint32_t bytesPerSec = (uint32_t) (SAMPLERATE * (BITRATE / 8)); // bytes per second
		uint16_t blockAlign = (uint16_t) (CHANNELS * (BITRATE / 8));          // 2=16-bit mono, 4=16-bit stereo
		uint16_t bitsPerSample = (uint16_t) BITRATE;      // Number of bits per sample
		/* "data" sub-chunk */
		uint8_t Subchunk2ID[4] = { 'd', 'a', 't', 'a' }; // "data"  string
		uint32_t Subchunk2Size = 0;                        // Sampled data length
	} wav_hdr;

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

	uint32_t datasize = (uint32_t)(pcmData.size() * (BITRATE / 8));
	wav_hdr wav;
	wav.ChunkSize = (datasize + sizeof(wav_hdr) - 8);
	wav.Subchunk2Size = datasize;

	ofstream out(filename, ios::binary);

	assert(sizeof(wav) == 44); //check header length
	out.write((char*)&wav, sizeof(wav)); //write header

	if (BITRATE == 16) {
		for (double& i : pcmData) {
			int16_t j = (int16_t)(clamp<double>(i, -1, 1) * INT16_MAX); //map to 16bit range & preserve symmetry (min = -32767)
			out.write((char*)&j, sizeof(int16_t)); //write data
		}
	}
	else if (BITRATE == 24) {
		for (double& i : pcmData) {
			int32_t j = (int32_t)(clamp<double>(i, -1, 1) * INT24_MAX);	//map to 24bit range & preserve symmetry (min = -8388607)
			out.write((char*)&j, sizeof(int8_t)*3); //write data
		}
	}



	if (out.fail()) {
		cout << "File write error!\n";
	}
	out.close();
}

class Osc : public Audio {
	int mode = 0;
	unsigned int length = 0, preLength = 0;
	double duration = 0, frequency = 1, amplitude = 0, preDelay = 0, period = 1;

	vector<double> sine();
	vector<double> triangle();
	vector<double> square();
	vector<double> saw();
	vector<double> sawr();
	vector<double> silence();
	vector<double> noise();
	public:
		void set();
}osc;

void Osc::set(){
	cout << "Osc mode? (1=sine | 2=triangle | 3=square | 4=saw | 5=reverse saw | 6=noise | 0=silence) ";
	while ( !(cin >> mode) || mode < 0 || mode > 6) {
		cin.clear();
		cin.ignore(1000, '\n');
		cout << "Invalid input.\nOsc mode? (1=sine | 2=triangle | 3=square | 4=saw | 5=reverse saw | 6=noise | 0=silence) ";
	}
	cout << "Duration? (max. 2000 seconds) ";
	while(!(cin >> duration) || duration < 0 || duration > 2000) {
		cin.clear();
		cin.ignore(1000, '\n');
		cout << "Invalid input.\nDuration? (max. 2000 seconds) ";
	}
	if (mode != 0) {
		if (mode != 6) {
			cout << "Frequency? (Hz) ";
			while (!(cin >> frequency) || frequency <= 0) {
				cin.clear();
				cin.ignore(1000, '\n');
				cout << "Invalid input.\nFrequency? (Hz) ";
			}
			if (frequency > SAMPLERATE / 2) {
				cout << "WARNING: Input too high, aliasing will occur.\n";
			}
		}
		cout << "Amplitude? (0-1000, %) ";
		while (!(cin >> amplitude) || amplitude < 0 || amplitude > 1000) {
			cin.clear();
			cin.ignore(1000, '\n');
			cout << "Invalid input.\nAmplitude? (0-1000, %) ";
		}
		if (amplitude < 5 && BITRATE == 16) {
			cout << "WARNING: Amplitude very low and dangerously close to noise floor.\n";
		}
		amplitude = amplitude*0.01; //percentage to 0-1 value
		cout << "Pre-delay? (max. 2000 seconds) ";
		while (!(cin >> preDelay) || preDelay < 0 || preDelay > 2000) {
			cin.clear();
			cin.ignore(1000, '\n');
			cout << "Invalid input.\nPre-delay? (max. 2000 seconds) ";
		}
	}

	length = (unsigned int)(duration * SAMPLERATE);
	preLength = (unsigned int)(preDelay * SAMPLERATE);
	period = (1 / frequency) * SAMPLERATE;

	switch (mode) {
		case 0:
			pcmData = silence();
			break;
		case 1:
			pcmData = sine();
			break;
		case 2:
			pcmData = triangle();
			break;
		case 3:
			pcmData = square();
			break;
		case 4:
			pcmData = saw();
			break;
		case 5:
			pcmData = sawr();
			break;
		case 6:
			pcmData = noise();
			break;
	}
	if (CHANNELS==2){
		vector<double> left = pcmData, right = pcmData;
		double pan = 0;
		pcmData.clear();
		cout << "Pan? (-100(L) - 100(R)) ";
		while (!(cin >> pan) || pan < -100 || pan > 100) {
			cin.clear();
			cin.ignore(1000, '\n');
			cout << "Invalid input.\nPan? (-100(L) - 100(R)) ";
		}
		transform(left.begin(), left.end(), left.begin(),
			[pan](double v){return v * ((clamp<double>(pan, 0, 100))-100) * -0.01;});

		transform(right.begin(), right.end(), right.begin(),
			[pan](double v){return v * ((clamp<double>(pan, -100, 0))+100) * 0.01;});

		for (unsigned int x=0; x<min(left.size(), right.size()); x++){
			pcmData.push_back(left[x]);
			pcmData.push_back(right[x]);
		}	
	}
	
}

vector<double> Osc::silence() {
	vector<double> result(preLength, 0);
	for (unsigned int x = 0; x < length; x++) {
		double samplevalue = 0;
		result.push_back(samplevalue);
	}
	return result;
}

vector<double> Osc::sine() {
	vector<double> result(preLength, 0);
	for (unsigned int x = 0; x < length; x ++) {
		double samplevalue = sin((x*2*M_PI)/period) * amplitude;
		result.push_back(samplevalue);
	}
	return result;
}

vector<double> Osc::triangle() {
	vector<double> result(preLength, 0);
	for (unsigned int x = 0; x < length; x ++) {
		double samplevalue = (4*amplitude/period)*abs(fmod((x+period-(period*0.25)),period)-(period*0.5))-amplitude;
		result.push_back(samplevalue);
	}
	return result;
}

vector<double> Osc::square() {
	vector<double> result(preLength, 0);
	for (unsigned int x = 0; x < length; x ++) {
		double samplevalue = sin((x*2*M_PI)/period);
		samplevalue = ((samplevalue >= 0) - (samplevalue < 0)) * amplitude;
		result.push_back(samplevalue);
	}
	return result;
}

vector<double> Osc::saw() {
	vector<double> result(preLength, 0);
	for (unsigned int x = 0; x < length; x ++) {
		double samplevalue = (((fmod(x, period))/period)-0.5)*2*amplitude;
		result.push_back(samplevalue);
	}
	return result;
}

vector<double> Osc::sawr() {
	vector<double> result(preLength, 0);
	for (unsigned int x = 0; x < length; x ++) {
		double samplevalue = -(((fmod(x, period))/period)-0.5)*2*amplitude;
		result.push_back(samplevalue);
	}
	return result;
}

vector<double> Osc::noise() {
	srand((unsigned int) time(nullptr));
	vector<double> result(preLength, 0);
	for (unsigned int x = 0; x < length; x++) {
		double samplevalue = ((rand() / (double)(RAND_MAX*0.5))-1)*amplitude;
		result.push_back(samplevalue);
	}
	return result;
}

int main() {
	audio.set();
	int oscCount = 0;
	char another = 'y';
	while (another == 'y') {
		oscCount ++;
		cout << "Oscillator #" << oscCount << ':' << endl;
		osc.set();
		final.sum(osc.pcmData);
		cout << "Add another oscillator? (y/n) ";
		while(!(cin >> another) || !(another == 'y' || another == 'n')) {
			cin.clear();
			cin.ignore(1000, '\n');
			cout << "Invalid input.\nAdd another oscillator? (y/n) ";
		}
	}

	final.makewav();
	
	return 0;
}
