#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <cmath>

//Generatorul de numere pseudo-aleatoare KISS al lui George Marsaglia:
//https://link.springer.com/content/pdf/10.1007%2Fs12095-017-0225-x.pdf
#define znew  ((z=36969*(z&65535)+(z>>16))<<16)
#define wnew  ((w=18000*(w&65535)+(w>>16))&65535)
#define MWC   (znew+wnew)
#define SHR3  (jsr=(jsr=(jsr=jsr^(jsr<<17))^(jsr>>13))^(jsr<<5))
#define CONG  (jcong=69069*jcong+1234567)
#define KISS  ((MWC^CONG)+SHR3)

using namespace std;

static unsigned long int z, w, jsr, jcong;

//Functia main trebuie apelata cu 2 parametrii:
//1. calea fisierului in care se va salva cheia secreta in format binar (acest fisier va fi //utilizat pentru criptare si decriptare)
//2. calea fisierului text in care se va salva cheia secreta (acest fisier este generate
//doar pentru a putea fi vizualizate mai usor valorile din cheia secreta).

//Valorile returnate de functia main:
//-1 = numar incorect de parametrii pentru functia main
//-2 = eroare la crearea fisierului binar pentru cheia secreta
//-3 = eroare la crearea fisierului text pentru cheia secreta

//Nu am mai tratat eventualele erori care pot sa apara la scriere in cele doua fisiere!
int main(int argc, char **argv)
{
    if(argc != 3)
    {
        cout << "Numar incorect de parametri: " << argc-1 << " (in loc de 2 parametri)!" << endl;
        return -1;
    }

    ofstream fout_binar(argv[1], ios::binary | ios::out);

    if(fout_binar.is_open() == false)
    {
        cout << "Fisierul binar " << argv[1] << " nu a putut fi creat!" << endl;
        return -2;
    }

    ofstream fout_text(argv[2]);

    if(fout_text.is_open() == false)
    {
        cout << "Fisierul binar " << argv[2] << " nu a putut fi creat!" << endl;
        return -3;
    }

    unsigned int i;

    //initializarea PRNG-ului KISS
    srand(time(NULL));

    z = rand();
    w = rand();
    jsr = rand();
    jcong = rand();

    unsigned int rnd;

    //numarul de preiteratii pentru cele 4 functii haotice, cuprins intre 1000 si 5000
    rnd = 1000 + KISS % 4001;

    fout_binar.write((char*)&rnd, sizeof(unsigned int));
    fout_text << rnd << endl;

    //vectorul de initializare IV (format din 3 numere naturale cuprinse intre 0 si 255)
    for(i = 0; i < 3; i++)
    {
        rnd = KISS % 256;

        fout_binar.write((char*)&rnd, sizeof(unsigned int));
        fout_text << rnd << " ";
    }
    fout_text << endl;

    //valorile initiale si parametrii celor 2 functii serpentina
    //valorile initiale pentru x trebuie sa fie cuprinse intre -PI/2 si PI/2
    //valorile initiale pentru y trebuie sa fie cuprinse intre -1/2 si 1/2
    //parametrii r trebuie sa fie mai mari decat 3 (aici sunt cuprinsi intre 3 si 20)
    double r, x, y;
    for(i = 0; i < 2; i++)
    {
        r = 3 + 17*(double)KISS/ULONG_MAX;
        x = -M_PI/2 + M_PI*(double)KISS/ULONG_MAX;
        y = -0.5 + (double)KISS/ULONG_MAX;

        fout_binar.write((char*)&r, sizeof(double));
        fout_binar.write((char*)&x, sizeof(double));
        fout_binar.write((char*)&y, sizeof(double));

        fout_text << setprecision(15) << r << " " << x << " " << y << endl;
    }

    fout_text.close();
    fout_binar.close();

    return 0;
}
