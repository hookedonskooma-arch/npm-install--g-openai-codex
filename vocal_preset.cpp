#include <iostream>
#include <string>
#include <vector>
using namespace std;

struct VocalPreset {
    string key;
    string scale = "minor";
    int retuneSpeed;
    int humanize;
    string vocalCharacter;
};

void printPreset(const VocalPreset& p) {
    cout << "\n=== CARTI POLISH / CHRIS GRAU VOCAL PRESET ===\n";
    cout << "Key: " << p.key << " " << p.scale << "\n";
    cout << "Retune Speed: " << p.retuneSpeed << " ms\n";
    cout << "Humanize: " << p.humanize << "%\n";
    cout << "Input Type: Tenor / Alto Male\n";
    cout << "Character: " << p.vocalCharacter << "\n\n";

    cout << "--- CHAIN ORDER ---\n";
    cout << "1. AutoTune: hard correction, minor scale only\n";
    cout << "2. EQ: HPF 80Hz, cut 300Hz, boost 4.5kHz, shelf 10kHz\n";
    cout << "3. Compressor: 4:1, attack 15ms, release auto, 4-6dB GR\n";
    cout << "4. DeEsser: 6.5kHz - 8kHz\n";
    cout << "5. Saturation: light tube drive\n";
    cout << "6. Delay Send: 1/4 note, 10% feedback, filtered\n";
    cout << "7. Reverb Send: plate, 0.8-1.2 sec, 50ms pre-delay\n";

    cout << "\n--- LAYERING ---\n";
    cout << "Lead: center, dry-ish, clean and tuned\n";
    cout << "Double L: pan -35, low volume\n";
    cout << "Double R: pan +35, low volume\n";
    cout << "Adlibs: wider, wetter, more AutoTune\n";
}

int main() {
    vector<string> minorKeys = {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

    cout << "Choose a minor key:\n";
    for (int i = 0; i < (int)minorKeys.size(); i++) {
        cout << i + 1 << ". " << minorKeys[i] << " minor\n";
    }

    int choice;
    cout << "\nEnter number: ";
    cin >> choice;

    if (choice < 1 || choice > (int)minorKeys.size()) {
        cout << "Invalid choice.\n";
        return 1;
    }

    VocalPreset preset;
    preset.key = minorKeys[choice - 1];
    preset.retuneSpeed = 3;
    preset.humanize = 10;
    preset.vocalCharacter =
        "70% natural Chris voice, 30% polished melodic trap sheen";

    printPreset(preset);

    return 0;
}
