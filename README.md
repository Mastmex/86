# 8086 Emulator

[🇷🇺 Русская версия](#rus) | [🇬🇧 English version](#eng)

# RUS

Простой эмулятор процессора Intel 8086 на C++.

## Описание

Эмулятор воспроизводит базовые функции микропроцессора Intel 8086, включая:
- Работу с регистрами
- Управление памятью
- Выполнение основных инструкций
- Обработку флагов

## Особенности

- Эмуляция регистров (AX, BX, CX, DX, SI, DI и др.)
- Адресация памяти через сегменты
- Основные инструкции (MOV, PUSH, POP, JMP и др.)
- Управление флагами (CF, PF, AF, ZF, SF и др.)
- Загрузка памяти из файлов

## Ограничения

Реализована только часть инструкций 8086. Не все функции полностью поддерживаются.

## Планы по развитию

- Добавление новых инструкций
- Улучшение отладки
- Оптимизация управления памятью
- Поддержка прерываний

## Сборка и запуск
### Создание информации сборки при помощи CMake

Для этого создается отдельная папка и прописывается команда
* `cmake -DCMAKE_BUILD_TYPE=Debug <Адрес директории проекта> -G "<Сборщик>"` для отладочной версии со всеми выводами
* `cmake -DCMAKE_BUILD_TYPE=Release <Адрес директории проекта> -G "<Сборщик>"` для релизной версии с оптимизацией

В моем случае я создаю в директории проекта директорию **build** и тогда `<Адрес директории проекта>`=`..` `<Сборщик>`=`MinGW Makefiles`

### Сборка при помощи CMake
Затем в директории сборки прописывается команда `cmake --build .`
### Запуск
Запуск выполняется командой `./main.exe` (`./main` в случае unix-систем)

# ENG

A simple Intel 8086 processor emulator written in C++.

## Description

This project emulates the functionality of an Intel 8086 microprocessor, including:
- Register operations
- Memory management
- Basic instruction set execution
- Flag handling

## Features

- Emulates core 8086 registers (AX, BX, CX, DX, SI, DI, etc.)
- Supports segment:offset memory addressing
- Implements basic instructions (MOV, PUSH, POP, JMP, etc.)
- Includes flag management (CF, PF, AF, ZF, SF, etc.)
- Memory loading from binary or hex files
- Error monitoring system

## Limitations
Currently implements a subset of 8086 instructions. Not all features are fully implemented.

## Future Work
- Expand instruction set coverage
- Add more comprehensive debugging
- Improve memory management
- Add interrupt handling

## Building and Running
### Generating Build Information with CMake

For this purpose, a separate folder is created and the following commands are used:
* `cmake -DCMAKE_BUILD_TYPE=Debug <Project directory path> -G "<Generator>"` for debug version with full outputs
* `cmake -DCMAKE_BUILD_TYPE=Release <Project directory path> -G "<Generator>"` for optimized release version

In my case, I create a **build** directory within the project directory, so:
* `<Project directory path>` becomes `..`
* `<Generator>` is `MinGW Makefiles`

### Building with CMake
Then in the build directory, run the command: `cmake --build .`

### Running
Execute the program with: `./main.exe` (`./main` on Unix systems)