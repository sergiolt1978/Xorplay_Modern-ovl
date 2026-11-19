
#define TESLA_INIT_IMPL
#include <tesla.hpp>
#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>

// Envía la orden al sysmodule escribiendo en command.txt
void sendCommand(const std::string& cmd) {
    FILE* f = fopen("sdmc:/config/xorplay/command.txt", "w");
    if (f) {
        fprintf(f, "%s\n", cmd.c_str());
        fclose(f);
    }
}

// Lee el estado actual desde status.txt
std::string readStatus() {
    std::ifstream file("sdmc:/config/xorplay/status.txt");
    if (!file.is_open()) {
        return "Sysmodule no detectado";
    }
    std::string line, result;
    int count = 0;
    while (std::getline(file, line) && count < 3) {
        if (!line.empty()) {
            if (!result.empty()) result += "\n";
            result += line;
            count++;
        }
    }
    return result.empty() ? "Listo" : result;
}

class XorPlayOverlayGui : public tsl::Gui {
private:
    float currentVolume = 1.0f;

public:
    XorPlayOverlayGui() {}

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("XorPlay Modern", "Control de musica");
        auto list = new tsl::elm::List();

        // Cuadro informativo con la canción o estado actual
        list->addItem(new tsl::elm::CategoryHeader("Estado actual"));
        list->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            std::string st = readStatus();
            renderer->drawString(st.c_str(), false, x + 10, y + 25, 16, renderer->a(0xFFFF));
        }), 75);

        // Controles de reproducción
        list->addItem(new tsl::elm::CategoryHeader("Reproduccion"));

        auto itemToggle = new tsl::elm::ListItem("Play / Pausa");
        itemToggle->setClickListener([](u64 keys) {
            if (keys & HidNpadButton_A) {
                sendCommand("toggle");
                return true;
            }
            return false;
        });
        list->addItem(itemToggle);

        auto itemNext = new tsl::elm::ListItem("Siguiente cancion");
        itemNext->setClickListener([](u64 keys) {
            if (keys & HidNpadButton_A) {
                sendCommand("next");
                return true;
            }
            return false;
        });
        list->addItem(itemNext);

        auto itemPrev = new tsl::elm::ListItem("Cancion anterior");
        itemPrev->setClickListener([](u64 keys) {
            if (keys & HidNpadButton_A) {
                sendCommand("prev");
                return true;
            }
            return false;
        });
        list->addItem(itemPrev);

        // Ajustes extra y volumen
        list->addItem(new tsl::elm::CategoryHeader("Opciones y Volumen"));

        auto itemVolUp = new tsl::elm::ListItem("Subir volumen (+10%)");
        itemVolUp->setClickListener([this](u64 keys) {
            if (keys & HidNpadButton_A) {
                currentVolume += 0.10f;
                if (currentVolume > 1.0f) currentVolume = 1.0f;
                char buf[32];
                snprintf(buf, sizeof(buf), "volume %.2f", currentVolume);
                sendCommand(buf);
                return true;
            }
            return false;
        });
        list->addItem(itemVolUp);

        auto itemVolDown = new tsl::elm::ListItem("Bajar volumen (-10%)");
        itemVolDown->setClickListener([this](u64 keys) {
            if (keys & HidNpadButton_A) {
                currentVolume -= 0.10f;
                if (currentVolume < 0.0f) currentVolume = 0.0f;
                char buf[32];
                snprintf(buf, sizeof(buf), "volume %.2f", currentVolume);
                sendCommand(buf);
                return true;
            }
            return false;
        });
        list->addItem(itemVolDown);

        auto itemShuffle = new tsl::elm::ListItem("Alternar Aleatorio (Shuffle)");
        itemShuffle->setClickListener([](u64 keys) {
            if (keys & HidNpadButton_A) {
                sendCommand("shuffle");
                return true;
            }
            return false;
        });
        list->addItem(itemShuffle);

        auto itemReload = new tsl::elm::ListItem("Recargar listas / config");
        itemReload->setClickListener([](u64 keys) {
            if (keys & HidNpadButton_A) {
                sendCommand("reload");
                return true;
            }
            return false;
        });
        list->addItem(itemReload);

        frame->setContent(list);
        return frame;
    }
};

class XorPlayOverlay : public tsl::Overlay {
public:
    virtual void initServices() override {
        fsdevMountSdmc();
    }

    virtual void exitServices() override {
        fsdevUnmountDevice("sdmc");
    }

    virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<XorPlayOverlayGui>();
    }
};

int main(int argc, char **argv) {
    return tsl::loop<XorPlayOverlay>(argc, argv);
}
