# Life Tracker⏱️
> ### Descriere
> 
> **Life Tracker** este o aplicație C++ de organizare personală all-in-one. 
> 
> Include:
> 1. **Money Tracker** (venituri, cheltuieli, abonamente și economii)
> 2. **Calendar** (sincronizat cu Google Calendar)
> 3. **Smart Task Tracker** (prioritizare prin comparații binare și niveluri de urgență)
> 4. **Roadmap Creator** (arbore vizual de obiective)
> ---
### Tema 0

- [x] Nume proiect
- [x] Scurtă descriere a temei alese, ce v-ați propus să implementați

## Tema 1

#### Cerințe
- [x] definirea a minim **3-4 clase** folosind compunere cu clasele definite de voi; moștenirile nu se iau în considerare aici
- [x] constructori de inițializare cu parametri pentru fiecare clasă
- [x] pentru o aceeași (singură) clasă: constructor de copiere, `operator=` de copiere, destructor
<!-- - [ ] pentru o altă clasă: constructor de mutare, `operator=` de mutare, destructor -->
<!-- - [ ] pentru o altă clasă: toate cele 5 funcții membru speciale -->
- [x] `operator<<` pentru **toate** clasele pentru afișare (`std::ostream`) folosind compunere de apeluri cu `operator<<`
- [x] cât mai multe `const` (unde este cazul) și funcții `private`
- [x] implementarea a minim 3 funcții membru publice pentru funcționalități netriviale specifice temei alese, dintre care cel puțin 1-2 funcții mai complexe
  - nu doar citiri/afișări sau adăugat/șters elemente într-un/dintr-un vector
- [x] scenariu de utilizare **cu sens** a claselor definite:
  - crearea de obiecte și apelarea tuturor funcțiilor membru publice în main
  - vor fi adăugate în fișierul `tastatura.txt` DOAR exemple de date de intrare de la tastatură (dacă există) pentru a acoperi cât mai multe cazuri; dacă aveți nevoie de date din fișiere, creați alte fișiere separat
- [x] minim 52-60% din codul propriu să fie C++, adică să aveți mai mult cod decât ce e inclus în template; testele nu se pun, nu vă luați după procentajul din interfața web; detalii la ore
- [x] `.gitattributes` configurat corect (nu trebuie făcute modificări aici)
- [x] tag de `git`: de exemplu `v0.1`
- [x] serviciu de integrare continuă (CI) cu **toate bifele**; exemplu: GitHub Actions
- [ ] code review #1 2 proiecte

## Tema 2

#### Cerințe
- [x] separarea codului din clase în `.h` (sau `.hpp`) și `.cpp`
- [x] moșteniri:
  - minim o clasă de bază și **3 clase derivate** din aceeași ierarhie; cele 3 derivate moștenesc aceeași clasă de bază
  - ierarhia trebuie să fie cu bază proprie, nu derivată dintr-o clasă predefinită
  - [x] funcții virtuale (pure) apelate prin pointeri de bază din clasa care conține atributul de tip pointer de bază
    - minim o funcție virtuală va fi **specifică temei** (i.e. nu simple citiri/afișări sau preluate din biblioteci i.e. draw/update/render)
    - constructori virtuali (clone): sunt necesari, dar nu se consideră funcții specifice temei
    - afișare virtuală, interfață non-virtuală
  - [x] apelarea constructorului din clasa de bază din constructori din derivate
  - [x] clasă cu atribut de tip pointer la o clasă de bază cu derivate; aici apelați funcțiile virtuale prin pointer de bază, eventual prin interfața non-virtuală din bază
    - [x] suprascris cc/op= pentru copieri/atribuiri corecte, copy and swap
    - [x] `dynamic_cast`/`std::dynamic_pointer_cast` pentru downcast cu sens
    - [x] smart pointers (recomandat, opțional)
- [x] excepții
  - [x] ierarhie proprie cu baza `std::exception` sau derivată din `std::exception`; minim **3** clase pentru erori specifice distincte
    - clasele de excepții trebuie să trateze categorii de erori distincte (exemplu de erori echivalente: citire fișiere cu diverse extensii)
  - [x] utilizare cu sens: de exemplu, `throw` în constructor (sau funcție care întoarce un obiect), `try`/`catch` în `main`
  - această ierarhie va fi complet independentă de ierarhia cu funcții virtuale
- [x] funcții și atribute `static`
- [x] STL
- [ ] cât mai multe `const`
- [ ] funcții *de nivel înalt*, de eliminat cât mai mulți getters/setters/funcții low-level
- [ ] minim 75-78% din codul propriu să fie C++; testele nu se pun, nu vă luați după procentajul din interfața web
- [ ] la sfârșit: commit separat cu adăugarea unei noi clase derivate fără a modifica restul codului, **pe lângă cele 3 derivate deja adăugate** din aceeași ierarhie
  - noua derivată nu poate fi una existentă care a fost ștearsă și adăugată din nou
  - noua derivată va fi integrată în codul existent (adică va fi folosită, nu adăugată doar ca să fie)
- [ ] tag de `git` pe commit cu **toate bifele**: de exemplu `v0.2`
- [ ] code review #2 2 proiecte

#### Ce s-a implementat (Tema 2, în lucru)
- **Ierarhia de alerte** (`app/reminder_rule.h`): baza abstractă `reminder_rule` și 3 derivate: `budget_alert` (sold sub prag, cheltuieli lunare peste limită, abonamente neacoperite), `deadline_alert` (task-uri scadente sau întârziate) și `habit_alert` (streak-uri în pericol, consistență scăzută).
  - funcția virtuală pură specifică temei: `analyze(const lifetracker&)`; afișarea virtuală: `print_details()`; constructorul virtual: `clone()`
  - interfață non-virtuală: `evaluate()` și `operator<<` din bază apelează funcțiile virtuale private
  - `reminder_center` ține regulile ca `std::vector<std::unique_ptr<reminder_rule>>`, are constructor de copiere prin `clone()`, `operator=` cu copy and swap și folosește `dynamic_cast<budget_alert*>` în `adapt_to_forecast()` ca să înăsprească doar regulile de buget când prognoza e negativă
  - `static`: `reminder_rule::live_rules()` (numărul de reguli existente) și `reminder_center::with_defaults()`
- **Excepții** (`app/lt_exceptions.h`): baza `lifetracker_error` (derivată din `std::runtime_error`) și `invalid_value_error` (configurare invalidă, aruncată din constructorii regulilor), `data_file_error` (fișiere de date lipsă sau goale) și `auth_error` (login greșit), prinse în `main` și în meniul TUI.
- În aplicație: opțiunea **[8] Alerte Inteligente** din meniul din terminal; în modul demo (`tastatura.txt`) alertele se afișează după sumar.

## Tema 3

#### Cerințe
- [ ] 2 șabloane de proiectare (design patterns)
- [ ] o clasă șablon cu sens; minim **2 instanțieri**
  - [ ] preferabil și o funcție șablon (template) cu sens; minim 2 instanțieri
- [ ] minim 80-90% din codul propriu să fie C++; testele nu se pun, nu vă luați după procentajul din interfața web
<!-- - [ ] o specializare pe funcție/clasă șablon -->
- [ ] tag de `git` pe commit cu **toate bifele**: de exemplu `v0.3` sau `v1.0`
- [ ] code review #3 2 proiecte

## Biblioteci folosite

#### Qt (opțional) - interfața grafică
[Qt](https://www.qt.io/) este un framework C++ multiplatformă (Linux, Windows, macOS) pentru aplicații cu interfață grafică. Proiectul folosește modulul **Qt Widgets**, care oferă componentele clasice de interfață: ferestre, butoane, câmpuri de text, tabele, liste și meniuri, plus mecanismul de semnale și sloturi prin care aplicația reacționează la acțiunile utilizatorului. Qt este disponibil sub licență open-source (LGPL/GPL).

Qt este **opțional**: la configurare, CMake caută Qt6, iar dacă nu îl găsește, Qt5. Dacă a găsit una dintre ele, se compilează cu macro-ul `USE_QT_GUI` și aplicația pornește cu interfața grafică, păstrând interfața din terminal ca variantă de rezervă. Dacă nu e instalat Qt, aplicația se compilează și funcționează doar cu interfața din terminal (TUI), deci proiectul nu depinde obligatoriu de Qt.

Instalare (exemple): `sudo apt install qt6-base-dev` pe Ubuntu/Debian, `brew install qt` pe macOS.

#### Biblioteca standard C++ (STL)
Containere și algoritmi (`<vector>`, `<map>`, `<algorithm>`), șiruri de caractere (`<string>`), fluxuri pentru consolă, fișiere și conversii text (`<iostream>`, `<fstream>`, `<sstream>`). Toată logica de domeniu (buget, calendar, task-uri, roadmap, obiceiuri, fitness) folosește doar biblioteca standard, fără dependențe externe.

#### CMake
Sistem de build care configurează proiectul, detectează opțional Qt și compilează logica de domeniu într-o bibliotecă statică (`oop`), folosită de executabil și de teste. Vezi secțiunea de mai jos.

## Instrucțiuni de compilare

Proiectul este configurat cu CMake.

Instrucțiuni pentru terminal:

1. Pasul de configurare
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
# sau ./scripts/cmake.sh configure
```

Sau pe Windows cu GCC folosind Git Bash:
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -G Ninja
# sau ./scripts/cmake.sh configure -g Ninja
```

Pentru a configura cu ASan, avem opțiunea `-DUSE_ASAN=ON` (nu merge pe Windows cu GCC):
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DUSE_ASAN=ON
# sau ./scripts/cmake.sh configure -e "-DUSE_ASAN=ON"
```


La acest pas putem cere să generăm fișiere de proiect pentru diverse medii de lucru.


2. Pasul de compilare
```sh
cmake --build build --config Debug --parallel 6
# sau ./scripts/cmake.sh build
```

Cu opțiunea `parallel` specificăm numărul de fișiere compilate în paralel.

3. Pasul de testare
```sh
ctest --test-dir build -C Debug --output-on-failure
# sau ./scripts/cmake.sh test
```

Testele sunt **condiție, nu punctaj**: CI verde include și testele. Nu se punctează nici numărul
lor, nici acoperirea (coverage). Testați logica de domeniu din `src/`, prin interfața publică din
`include/`, fără `std::cin` și fără căi absolute. Ce e greu de testat — UI complex, evenimente
aleatoare, un server real — rămâne în `app/` și se verifică prin smoke tests și la demo.

4. Pasul de instalare (opțional)
```sh
cmake --install build --config Debug --prefix install_dir
# sau ./scripts/cmake.sh install
```

Vezi și [`scripts/cmake.sh`](scripts/cmake.sh).

Observație: folderele `build/` și `install_dir/` sunt adăugate în fișierul `.gitignore` deoarece
conțin fișiere generate și nu ne ajută să le versionăm.

## Instrucțiuni pentru a rula executabilul

Există mai multe variante:

1. Din directorul de build (implicit `build`). Executabilul se află la locația `./build/oop_main` după ce a fost rulat pasul de compilare al proiectului (`./scripts/cmake.sh build` - pasul 2 de mai sus).

```sh
./build/oop_main
```

2. Din directorul `install_dir`. Executabilul se află la locația `./install_dir/bin/oop_main` după ce a fost rulat pasul de instalare (`./scripts/cmake.sh install` - pasul 4 de mai sus).

```sh
./install_dir/bin/oop_main
```

3. Rularea programului folosind Valgrind se poate face executând script-ul `./scripts/run_valgrind.sh` din rădăcina proiectului. Pe Windows acest script se poate rula folosind WSL (Windows Subsystem for Linux). Valgrind se poate rula în modul interactiv folosind: `RUN_INTERACTIVE=true ./scripts/run_valgrind.sh`

Implicit, nu se rulează interactiv, iar datele pentru `std::cin` sunt preluate din fișierul `tastatura.txt`.

```sh
RUN_INTERACTIVE=true ./scripts/run_valgrind.sh
# sau
./scripts/run_valgrind.sh
```

Valgrind rulează și testele.

4. Pentru a rula executabilul folosind ASan, este nevoie ca la pasul de configurare (vezi mai sus) să fie activat acest sanitizer. Ar trebui să meargă pe macOS și Linux. Pentru Windows, ar merge doar cu MSVC (nerecomandat).

Comanda este aceeași ca la pasul 1 sau 2. Nu merge combinat cu Valgrind.

```sh
./build/oop_main
./build/oop_test
# sau
./install_dir/bin/oop_main
./install_dir/bin/oop_test
```

## License

The project is licensed under [AGPLv3](LICENSE).

The [template repository](https://github.com/mcmarius/oop-template) itself is licensed under [Unlicense](LICENSE.template).

## Resurse

- adăugați trimiteri **detaliate** către resursele externe care v-au ajutat sau pe care le-ați folosit
- Qt Widgets, prezentare generală a modulului: https://doc.qt.io/qt-6/qtwidgets-index.html
- Qt cu CMake (`find_package(Qt6 ...)`, linkarea `Qt6::Widgets`): https://doc.qt.io/qt-6/cmake-get-started.html
- Manualul CMake pentru Qt: https://cmake.org/cmake/help/latest/manual/cmake-qt.7.html
- Documentația CMake pentru `find_package` (folosit pentru detectarea opțională a Qt): https://cmake.org/cmake/help/latest/command/find_package.html
- Referința limbajului și a bibliotecii standard C++: https://en.cppreference.com/
- Formatul iCalendar (`.ics`), folosit pentru exportul evenimentelor în Google Calendar: https://datatracker.ietf.org/doc/html/rfc5545
