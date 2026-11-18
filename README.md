# T-TUNE — Pitch Correction Plugin

## Сборка
1. Установить Visual Studio 2022 (C++) / Xcode / g++
2. Клонировать JUCE: `git clone https://github.com/juce-framework/JUCE.git`
3. `cmake -B build -DCMAKE_BUILD_TYPE=Release`
4. `cmake --build build --config Release`
5. VST3 будет в `build/T_TUNE_artefacts/Release/VST3/`

## Установка в FL Studio
- Windows: `C:\Program Files\Common Files\VST3\`
- macOS: `~/Library/Audio/Plug-Ins/VST3/`
- FL Studio: Options → Manage plugins → Find more plugins
