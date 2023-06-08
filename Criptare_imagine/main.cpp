#include <cmath>
#include <climits>
#include <set>
#include <iostream>
#include <fstream>

#include "EasyBMP.h"

using namespace std;

//Functia serpentina
void serpentina(double r , double& x , double& y)
{
    x = atan(1/tan(r*x));
	y = sin(r*y)*cos(r*y);
}

//Functia main trebuie apelata cu 3 parametrii, respectiv:
//1. calea imaginii initiale (imaginea in clar)
//2. calea imaginii criptate
//3. calea fisierului binar care contine cheia secreta

//Valorile returnate de functia main:
//-1 = numar incorect de parametrii pentru functia main
//-2 = eroare la deschiderea fisierului binar care contine cheia secreta

//Nu am putut sa tratez erorile de deschidere/creare ale imaginilor, deoarece
//ele sunt realizate de metode din biblioteca EasyBMP care nu sunt documentate!

//Nu am tratat cazul in care fisierul binar cu cheia secreta nu are formatul corect!

int main(int argc, char **argv)
{
    if(argc != 4)
    {
        cout << "Numar incorect de parametri: " << argc-1 << " (in loc de 3 parametri)!" << endl;
        return -1;
    }

    //imaginea initiala
	BMP img_plain;

	img_plain.ReadFromFile(argv[1]);

	unsigned int imgHeight = img_plain.TellHeight();
	unsigned int imgWidth = img_plain.TellWidth();
	unsigned int noPixels = imgHeight * imgWidth;

	//imaginea in care se vor salva pixelii permutati
	BMP img_permuted;

	img_permuted.SetSize(imgWidth,imgHeight);
	img_permuted.SetBitDepth(img_plain.TellBitDepth());

	//imaginea criptata
	BMP img_encrypted;

	img_encrypted.SetSize(imgWidth,imgHeight);
	img_encrypted.SetBitDepth(img_plain.TellBitDepth());

	//tabloul unidimensional in care se va genera o permutare aleatoare p folosind algoritmul din lucrare
	unsigned int *p = new unsigned int[3*noPixels + 1];
	//tablou unidimensional de marcaje
	bool *L = new bool[3*noPixels + 1];

	//parametrii celor 2 functii serpentina
	double *r = new double[2];
	//valorile initiale ale celor 2 functii serpentina
	double *x = new double[2];
	double *y = new double[2];

	ifstream fin_cheia_secreta(argv[3], ios::binary | ios::in);

	if(fin_cheia_secreta.is_open() == false)
    {
        cout << "Eroare la deschiderea fisierului binar care contine cheia secreta!" << endl;
        return -2;
    }

    //citirea componentelor cheii secrete

    //numarul de preiteratii pentru cele 2 functii serpentina
	unsigned int no_preiterations;
    fin_cheia_secreta.read((char*)&no_preiterations, sizeof(unsigned int));

    //pixelul corespunzator IV
	RGBApixel pixel_iv;
	unsigned int aux;
	fin_cheia_secreta.read((char*)&aux, sizeof(unsigned int));
	pixel_iv.Red = aux;
	fin_cheia_secreta.read((char*)&aux, sizeof(unsigned int));
	pixel_iv.Green = aux;
	fin_cheia_secreta.read((char*)&aux, sizeof(unsigned int));
	pixel_iv.Blue = aux;
	pixel_iv.Alpha = 0;

	//parametrii si valorile initiale ale celor 2 functii serpentina
    for(unsigned int i = 0; i < 2; i++)
    {
        fin_cheia_secreta.read((char*)&r[i], sizeof(double));
        r[i] = pow(2, r[i]);
        fin_cheia_secreta.read((char*)&x[i], sizeof(double));
        fin_cheia_secreta.read((char*)&y[i], sizeof(double));
    }

    fin_cheia_secreta.close();

	//preiterarea celor 2 functii serpentina
	for(unsigned int i = 0; i < 2; i++)
		for(unsigned int j = 0; j < no_preiterations; j++)
			serpentina(r[i], x[i], y[i]);

	//generarea permutarii aleatoare p
	//unusedValues = un obiect de tip set cu elemente de tip intreg (vezi STL)
	set<int> unusedValues;

	//initial, toate valorile cuprinse intre 1 si n sunt nefolosite
	for(unsigned int i = 1; i <= 3*noPixels; i++)
    {
        unusedValues.insert(i);
        L[i] = false;
    }

	for(unsigned int k = 0; k < 3*noPixels; k++)
	{
		//se discretizeaza valoarea primei ramuri a primului sistem dinamic de tip serpentina,
		//prin inmultire cu 10^15 si apoi parte intreaga
		unsigned long long aux_ull_x = (unsigned long long)((0.5 + x[0]/M_PI) * 1000000000000000.0);
		unsigned long long aux_ull_y = (unsigned long long)((0.5 + y[0]) * 1000000000000000.0);

		//se scaleaza valoarea intreaga obtinuta prin discretizare astfel incat sa fie cuprinsa intre 1 si nr. de pixeli ai imaginii
		unsigned int uix = 1 + (unsigned int)(aux_ull_x % noPixels);
		unsigned int uiy = 1 + (unsigned int)(aux_ull_y % noPixels);

		//daca valoarea curenta nu a fost deja folosita, o atribuim elementului curent al permutarii
		if(L[uix] == false)
			p[k] = uix;
		else
			//in caz contrar, in functie de rezultatul compararii valorii curente a celei de-a doua ramuri a primei functii de tip serpentina
			//selectam fie valoarea minima nefolosita pana in acel moment
			if((uix >> (uiy % 32)) % 2 == 0)
				p[k] = *unusedValues.begin();
			//fie valoarea maxima nefolosita pana in acel moment
			else
				p[k] = *(--unusedValues.end());

		//marcam valoarea curenta din permutare ca fiind utilizata
		L[p[k]] = true;
		//si o stergem din multimea valorilor neutilizate
		unusedValues.erase(p[k]);

		serpentina(r[0], x[0], y[0]);
	}

	delete[] L;

	//se extrag octetii imaginii initiale img_plain, corespunzatori canalelor de culoare,
	//in tabloul img_bytes
	unsigned char* img_bytes = new unsigned char[3*noPixels];
	unsigned int no_bytes = 0;

	for(unsigned int row = 0; row < imgHeight; row++)
        for(unsigned int column = 0; column < imgWidth; column++)
        {
            RGBApixel pixel_curent;

            pixel_curent = img_plain.GetPixel(column, row);
            img_bytes[no_bytes++] = pixel_curent.Red;
            img_bytes[no_bytes++] = pixel_curent.Green;
            img_bytes[no_bytes++] = pixel_curent.Blue;
        }

	//se permuta octetii imaginii initiale din img_bytes in tabloul img_bytes_permuted
	unsigned char* img_bytes_permuted = new unsigned char[3*noPixels];
	for(unsigned int k = 0; k < 3*noPixels; k++)
        img_bytes_permuted[p[k]-1] = img_bytes[k];

	delete[] p;
	delete[] img_bytes;

	//se construieste imaginea permutata, pixel cu pixel, din octetii permutati
	no_bytes = 0;
	for(unsigned int row = 0; row < imgHeight; row++)
        for(unsigned int column = 0; column < imgWidth; column++)
        {
            RGBApixel pixel_curent;

            pixel_curent.Red = img_bytes_permuted[no_bytes++];
            pixel_curent.Green = img_bytes_permuted[no_bytes++];
            pixel_curent.Blue = img_bytes_permuted[no_bytes++];

            img_permuted.SetPixel(column, row, pixel_curent);
        }

	delete[] img_bytes_permuted;

	RGBApixel pixel_encrypted , pixel_permuted, pixel_aux;
	unsigned char *pks_1 , *pks_2;

	//XOR-are pixelilor conform formulei din lucrare
	for(unsigned int k = 0; k < noPixels; k++)
	{
        unsigned long long aux_ull_x = (unsigned long long)((0.5 + x[1]/M_PI) * 1000000000000000.0);
		unsigned long long aux_ull_y = (unsigned long long)((0.5 + y[1]) * 1000000000000000.0);

		unsigned int ks_1 = (unsigned int)(aux_ull_x);
		unsigned int ks_2 = (unsigned int)(aux_ull_y);

		pixel_permuted = img_permuted.GetPixel(k % imgWidth , k / imgWidth);

		if(k == 0)
			pixel_aux = pixel_iv;
		else
			pixel_aux = img_encrypted.GetPixel((k - 1) % imgWidth , (k - 1) / imgWidth);

		pks_1 = (unsigned char *)(&ks_1);
		pks_2 = (unsigned char *)(&ks_2);

		pixel_encrypted.Red = pixel_aux.Red ^ pixel_permuted.Red ^ (*(pks_1 + 1)) ^  (*(pks_2 + 1));
		pixel_encrypted.Green = pixel_permuted.Green ^ (*(pks_1 + 2)) ^ pixel_aux.Green ^ (*(pks_2 + 2));
		pixel_encrypted.Blue = pixel_permuted.Blue ^ (*(pks_1 + 3)) ^ pixel_aux.Blue ^ (*(pks_2 + 3));
		pixel_encrypted.Alpha = 0;

		img_encrypted.SetPixel(k % imgWidth, k / imgWidth , pixel_encrypted);

        serpentina(r[1], x[1], y[1]);
	}

	img_encrypted.WriteToFile(argv[2]);

	return 0;
}
