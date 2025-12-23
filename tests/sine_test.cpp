// simple_sine.cpp - Génère une sinusoïde 440Hz pendant 1 seconde
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <cstdint>

int main() {
    std::cout << "Génération d'une sinusoïde 440Hz pour 1 seconde...\n";
    
    // Paramètres audio
    const float SAMPLE_RATE = 44100.0f;  // 44.1kHz
    const float FREQUENCY = 440.0f;      // La 440
    const float DURATION = 1.0f;         // 1 seconde
    const float AMPLITUDE = 0.5f;        // Volume à 50%
    
    // Calculer le nombre d'échantillons
    int numSamples = static_cast<int>(SAMPLE_RATE * DURATION);
    
    // Générer les échantillons
    std::vector<float> samples;
    samples.reserve(numSamples);
    
    for (int i = 0; i < numSamples; i++) {
        float time = i / SAMPLE_RATE;
        float sample = AMPLITUDE * sin(2.0f * M_PI * FREQUENCY * time);
        samples.push_back(sample);
    }
    
    // Écrire le fichier WAV (format 32-bit float)
    std::ofstream wavFile("sine_440.wav", std::ios::binary);
    
    if (!wavFile) {
        std::cerr << "Erreur: Impossible de créer le fichier WAV\n";
        return 1;
    }
    
    // En-tête WAV
    struct WavHeader {
        // RIFF Chunk
        char riff[4] = {'R', 'I', 'F', 'F'};
        uint32_t chunkSize;                 // Taille fichier - 8
        char wave[4] = {'W', 'A', 'V', 'E'};
        
        // fmt Subchunk
        char fmt[4] = {'f', 'm', 't', ' '};
        uint32_t subchunk1Size = 16;        // 16 pour PCM
        uint16_t audioFormat = 3;           // 3 = IEEE float
        uint16_t numChannels = 1;           // Mono
        uint32_t sampleRate;                // 44100
        uint32_t byteRate;                  // sampleRate * numChannels * bitsPerSample/8
        uint16_t blockAlign;                // numChannels * bitsPerSample/8
        uint16_t bitsPerSample = 32;        // 32-bit float
        
        // data Subchunk
        char data[4] = {'d', 'a', 't', 'a'};
        uint32_t subchunk2Size;             // Nombre d'octets de données
    };
    
    WavHeader header;
    header.sampleRate = static_cast<uint32_t>(SAMPLE_RATE);
    header.byteRate = header.sampleRate * header.numChannels * header.bitsPerSample / 8;
    header.blockAlign = header.numChannels * header.bitsPerSample / 8;
    header.subchunk2Size = samples.size() * sizeof(float);
    header.chunkSize = 36 + header.subchunk2Size;
    
    // Écrire l'en-tête
    wavFile.write(reinterpret_cast<char*>(&header), sizeof(WavHeader));
    
    // Écrire les données audio
    wavFile.write(reinterpret_cast<const char*>(samples.data()), 
                  samples.size() * sizeof(float));
    
    wavFile.close();
    
    std::cout << "Fichier 'sine_440.wav' généré avec succès !\n";
    std::cout << "Taille: " << samples.size() << " échantillons\n";
    std::cout << "Durée: " << DURATION << " secondes\n";
    std::cout << "Fréquence: " << FREQUENCY << " Hz\n";
    
    return 0;
}