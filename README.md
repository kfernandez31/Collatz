# Zadanie zaliczeniowe 3: Problem Collatza

## Wprowadzenie
Problem Collatza, zwany też czasami problemem $3x + 1$ to słynny otwarty problem z dziedziny informatyki. W skrócie, chodzi o odpowiedź na pytanie czy następująca pętla się zatrzyma dla każdego $n \geq 2$:
```cpp
while (n != 1)
    if (n % 2 == 0) { n = n / 2; }
    else { n = 3 * n + 1; }
```

Pewnego dnia spacerując ulicą Dobrą, zauważył go na ścianie Biblioteki Uniwersytetu Warszawskiego znany milioner - pan Bajtazar. Postanowił raz na zawsze rozwiązać tę zagadkę. W tym celu zorganizował konkurs, którego celem było wyłonienie zespołu, który będzie potrafił rozwiązywać instancję problemu jak najszybciej.

Do konkursu zgłosiło się aż **13 zespołów**, a każdy z nich postanowił podejść do problemu trochę inaczej. Celem zadania jest zaimplementowanie konkursu i **porównanie wyników zespołów**.
Opis zadania

Zadaniem studenta jest uzupełnienie kodu dołączonego w pliku `collatz.zip`, a następnie przygotowanie raportu porównującego wyniki zespołów. W pliku main.cpp znajduje się kod testujący każdy z zespołów w szeregu eksperymentów. Aby testy zakończyły się z sukcesem należy uzupełnić implementację zespołów w pliku teams.cpp, uwzględniając zawartość pliku teams.hpp. Każdy zespół powinien wywoływać w odpowiedni dla niej sposób funkcję `calcCollatz(...)` z pliku `collatz.hpp`. Zespoły powinny być zaimplementowane w następujący sposób:

- `TeamSolo` to jednoosobowy zespół wykorzystujący jeden proces i jeden wątek. Jego implementacji nie trzeba modyfikować. Należy go użyć jako punktu odniesienia dla pozostałych zespołów.
- `TeamNewThreads` powinien tworzyć nowy wątek dla każdego wywołania calcCollatz, jednak nie więcej niż `getSize()` wątków jednocześnie.
- `TeamConstThreads` powinien utworzyć `getSize()` wątków, a każdy z wątków powinien wykonać podobną, zadaną z góry ilość pracy.
- `TeamPool` powinien użyć dołączonej puli wątków `cxxpool::thread_pool`. Dokumentacja puli znajduje się na [githubie](https://github.com/bloomen/cxxpool)
- `TeamNewProcesses` powinien tworzyć nowy proces dla każdego wywołania `calcCollatz`, jednak nie więcej niż `getSize()` procesów jednocześnie.
- `TeamConstProcesses` powinien tworzyć `getSize()` procesów, a każdy z procesów powinien wykonać podobną, zadaną z góry ilość pracy.
- `TeamAsync` powinien użyć mechanizmu `std::async`. W przeciwieństwie do pozostałych drużyn nie limituje on zasobów.

Każdy ze współbieżnych zespołów ma swojego klona - odpowiadający mu zespół `X` (np. `TeamPoolX`, `TeamAsyncX`). Zespoły `X` nie muszą używać funkcji `calcCollatz` z pliku `collatz.hpp`, mogą użyć swojej własnej implementacji. W szczególności oczekuje się, że zespoły działające w obrębie jednego procesu wykorzystają dzieloną strukturę `SharedResults` (plik `sharedresults.hpp`). `SharedResults` powinna przechowywać wybrane wyniki częściowe, aby (przynajmniej w teorii) przyspieszyć obliczenia. Ustalenie interfejsu i implementacja struktury `SharedResults` to część zadania. W przypadku zespołów tworzących procesów wymagana jest komunikacja między procesami. Można np. użyć mechanizmów takich jak pamięć dzielona poznanych na laboratorium w języku C.

Podsumowanie wyników konkursu powinno zostać opisane w *raporcie*, który przedstawia czas działania każdego zespołu w różnych scenariuszach. Czas działania powinien być przedstawiony na czytelnych wykresach, na podstawie danych zbieranych przez obiekty klasy `DefaultTimer`. Dla każdego konkursu uzasadnij dlaczego konkretne drużyny były szybsze, a inne wolniejsze. Rozwiązania wielowątkowe powinny w uzasadnionych przypadkach być istotnie szybsze, jednak nie należy spodziewać się liniowego przyspieszenia w każdym z przypadków. Opisując wyniki uwzględnij rezultaty "`CalcCollatzSoloTimer`" dla różnych konkursów. W uzasadnieniu możesz również uwzględnić swoje własne timery, jeśli je dodasz. Eksperymenty należy przeprowadzić w dwóch różnych środowiskach np. komputer stacjonarny / laptop i maszyna students. Zadbaj o opis środowiska w raporcie, w szczególności model procesora wraz z liczbą rdzeni. Zwróć uwagę na różnice w wynikach które pojawią się przy wielokrotnym uruchamianiu programu. Idealny raport powinien zmieścić się na dwóch stronach a4.

## Reguły implementacji
- Zadanie powinno składać się z paczki `ab123456.zip` (inicjały + numer indeksu), która zawiera:
    - Wszystkie pliki niezbędne do skompilowania i bezbłędnego uruchomienia programu `./main`
    - Plik `ab123456.pdf`, zawierający raport
- Nadesłany kod musi kompilować się (poleceniem `make`) i działać na maszynie students.
- Istnieje możliwość nadsyłania części rozwiązania. W takim wypadku należy wykomentować ("`//`") niedziałające testy w pliku main.cpp oraz napisać o tym w raporcie.
- Nie należy używać bibliotek innych niż: biblioteki załączone w paczce, biblioteka standardowa i biblioteka systemowa.
- Należy uzupełnić funkcje w pliku `teams.cpp`, implementację klasy `SharedResults` oraz procesu `new_process.cpp`.
- Jeśli wymagane, należy dodać brakujące `#include`. Należy też implementować własne funkcje pomocnicze.
- W uzasadnionych przypadkach można też modyfikować pozostałą część kodu, jednak należy to uzasadnić w raporcie.
- Oceniana będzie poprawność i wydajność kodu oraz raport.

## Ważne porady
- Uruchomienie wszystkich testów zajmuje bardzo dużo czasu. Aby przyspieszyć rozwiązywanie zadania podczas implementacji uruchamiaj tylko podzbiór testów (pozostałe np. zakomentuj).
- Naprawdę sprawdź swoje rozwiązanie na students. Co roku nadsyłane są rozwiązania, które tracą istotną część punktów, ponieważ ich autorzy nie poświęcili na koniec chwili na ich przetestowanie.

## Pytania i odpowiedzi

Pytania należy zadawać na adres e-mail aj334557@mimuw.edu.pl.

- Pytanie: Co w przypadku gdy `TeamAsync` lub `TeamAsyncX` generuje błędy bad_alloc na maszynie students? [08.02.2022]

    Odpowiedź: Z uwagi na niezbyt wysoki limit na "`data seg size`" (`ulimit -d`) na maszynach students niektóre rozwiązania mogą powodować zwracanie błędów `bad_alloc` przy użyciu `std::async`. Osoby które na ten błąd nie trafiły lub poradziły sobie z nim samodzielnie nie muszą robić w tej kwestii nic, bez obawy o utratę punktów (niezależnie od tego jakie rozwiązanie problemu zostało zastosowane). W przypadku wystąpienia problemu proponuję jedno z następujących rozwiązań: 1) tymczasowe obniżenie rozmiaru stosu do 128 kbytes (wywołanie "`ulimit -s 128`" spowoduje obniżenie tego limitu w ramach shella) lub 2) użycie `std::launch::deferred` w przypadku testów które powodują `bad_alloc` i opisanie w raporcie wpływu wykorzystania `deferred`.
