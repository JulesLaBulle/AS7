#ifndef WAV_WRITER_H
#define WAV_WRITER_H

#include <fstream>
#include <vector>
#include <cstdint>
#include <string>

class WavWriter {
private:
    std::ofstream file;

    WavWriter() = default;
    
    // Écrit un entier 32-bit en little-endian
    void writeUint32(uint32_t value) {
        file.put(static_cast<char>(value & 0xFF));
        file.put(static_cast<char>((value >> 8) & 0xFF));
        file.put(static_cast<char>((value >> 16) & 0xFF));
        file.put(static_cast<char>((value >> 24) & 0xFF));
    }
    
    // Écrit un entier 16-bit en little-endian
    void writeUint16(uint16_t value) {
        file.put(static_cast<char>(value & 0xFF));
        file.put(static_cast<char>((value >> 8) & 0xFF));
    }
    
public:
    /**
     * Ouvre un fichier WAV pour écriture
     * @param filename Nom du fichier de sortie
     * @param sampleRate Fréquence d'échantillonnage (ex: 44100)
     * @return true si succès, false sinon
     */
    bool open(const std::string& filename, uint32_t sampleRate = 44100) {
        file.open(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Écrit l'en-tête WAV (format float 32-bit)
        const char riff[] = {'R', 'I', 'F', 'F'};
        file.write(riff, 4);
        writeUint32(0);  // Taille fichier - 8 (à remplir à la fin)
        
        const char wave[] = {'W', 'A', 'V', 'E'};
        file.write(wave, 4);
        
        const char fmt[] = {'f', 'm', 't', ' '};
        file.write(fmt, 4);
        writeUint32(16);  // Taille du chunk fmt (16 pour PCM)
        
        writeUint16(3);   // Format audio (3 = float 32-bit IEEE)
        writeUint16(1);   // Nombre de canaux (mono)
        writeUint32(sampleRate);  // Fréquence d'échantillonnage
        writeUint32(sampleRate * 4);  // Byte rate (sampleRate * canaux * 4)
        writeUint16(4);   // Block align (canaux * 4)
        writeUint16(32);  // Bits par échantillon (32 pour float)
        
        const char data[] = {'d', 'a', 't', 'a'};
        file.write(data, 4);
        writeUint32(0);  // Taille données (à remplir à la fin)
        
        return true;
    }
    
    /**
     * Écrit des échantillons audio dans le fichier
     * @param samples Vecteur d'échantillons (float entre -1.0 et 1.0)
     */
    void writeSamples(const std::vector<float>& samples) {
        file.write(reinterpret_cast<const char*>(samples.data()), 
                   samples.size() * sizeof(float));
    }
    
    /**
     * Ferme le fichier et met à jour les tailles dans l'en-tête
     */
    void close() {
        if (!file.is_open()) {
            return;
        }
        
        // Position actuelle = fin des données
        std::streampos dataEnd = file.tellp();
        
        // Calcule la taille des données audio
        uint32_t dataSize = static_cast<uint32_t>(dataEnd) - 44;
        
        // Retourne au début pour écrire les tailles
        file.seekp(4, std::ios::beg);
        writeUint32(dataSize + 36);  // Taille fichier - 8
        
        file.seekp(40, std::ios::beg);
        writeUint32(dataSize);  // Taille données audio
        
        file.close();
    }
    
    /**
     * Méthode statique utilitaire pour écrire un fichier en une ligne
     */
    static bool writeFile(const std::string& filename, 
                         const std::vector<float>& samples,
                         uint32_t sampleRate = 44100) {
        WavWriter writer;
        if (!writer.open(filename, sampleRate)) {
            return false;
        }
        
        writer.writeSamples(samples);
        writer.close();
        return true;
    }
};

#endif