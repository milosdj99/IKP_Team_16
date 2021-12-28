// MainProcess.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

int main()
{
    int unos;

    srand(time(0));

    while (true) {
        do {
            printf("1.Manuelni unos");
            printf("2.Automatski unos");

            scanf("%d", &unos);

        } while (unos != 1 && unos != 2);



        int podatak;

        if (unos == 1) {
            printf("Unesite podatak: (pritisnite x za povratak)");

            scanf("%d", &podatak);
            if (podatak == 'x') {
                continue;
            }

            //send podatak
        }
        else if (unos == 2){
            podatak = rand();
            //send podatak
        }
    }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
