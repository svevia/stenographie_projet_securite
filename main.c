#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

typedef unsigned short uint16;
typedef unsigned int uint32;

typedef struct
{
	uint16 signature;
	uint32 taille_fichier;
	uint32 reserve;
	uint32 offset_donnees;
} entete_fichier;

typedef struct
{
	uint32 taille_entete;
	uint32 largeur;
	uint32 hauteur;
	uint16 nombre_plans;
	uint16 profondeur;
	uint32 compression;
	uint32 taille_donnees_image;
	uint32 resolution_horizontale;
	uint32 resolution_verticale;
	uint32 taille_palette; /* en nombre de couleurs */
	uint32 nombre_de_couleurs_importantes; /* 0 */
} entete_bitmap;

typedef struct
{
	entete_fichier fichier;
	entete_bitmap bitmap;
} entete_bmp;


int lire_deux_octets(int fd, uint16 *val){
	return read(fd,val,2);
}

int lire_quatre_octets(int fd, uint32 *val){
	return read(fd,val,4);
}

int lire_entete(int de, entete_bmp *entete){
	
	//entete fichier
	lire_deux_octets(de, &entete->fichier.signature);
	lire_quatre_octets(de, &entete->fichier.taille_fichier);
	lire_quatre_octets(de, &entete->fichier.reserve);
	lire_quatre_octets(de, &entete->fichier.offset_donnees);

	//entete bitmap
	lire_quatre_octets(de, &entete->bitmap.taille_entete);
	lire_quatre_octets(de, &entete->bitmap.largeur);
	lire_quatre_octets(de, &entete->bitmap.hauteur);
	lire_deux_octets(de, &entete->bitmap.nombre_plans);
	lire_deux_octets(de, &entete->bitmap.profondeur);
	lire_quatre_octets(de, &entete->bitmap.compression);
	lire_quatre_octets(de, &entete->bitmap.taille_donnees_image);
	lire_quatre_octets(de, &entete->bitmap.resolution_horizontale);
	lire_quatre_octets(de, &entete->bitmap.resolution_verticale);
	lire_quatre_octets(de, &entete->bitmap.taille_palette);
	lire_quatre_octets(de, &entete->bitmap.nombre_de_couleurs_importantes);
	return 0;
}

int ecrire_deux_octets(int fd, uint16 *val){
	return write(fd,val,2);
}

int ecrire_quatre_octets(int fd, uint32 *val){
	return write(fd,val,4);
}

int ecrire_texte(int fd, char *val){
	int byte[7];
	int j;
	for(j= 0;&val[j] != 0; j++){
		printf("%s\n",&val[j]);
		int i = 0;
		for(i = 7; i >= 0; --i){
        	byte[i]= (*byte & 1 << i) ?  1 : 0;
		}
    }
	return write(fd,byte,1);
}



int ecrire_entete(int fd, entete_bmp *entete){
	
	//entete fichier
	ecrire_deux_octets(fd, &entete->fichier.signature);
	ecrire_quatre_octets(fd, &entete->fichier.taille_fichier);
	ecrire_quatre_octets(fd, &entete->fichier.reserve);
	ecrire_quatre_octets(fd, &entete->fichier.offset_donnees);

	//entete bitmap
	ecrire_quatre_octets(fd, &entete->bitmap.taille_entete);
	ecrire_quatre_octets(fd, &entete->bitmap.largeur);
	ecrire_quatre_octets(fd, &entete->bitmap.hauteur);
	ecrire_deux_octets(fd, &entete->bitmap.nombre_plans);
	ecrire_deux_octets(fd, &entete->bitmap.profondeur);
	ecrire_quatre_octets(fd, &entete->bitmap.compression);
	ecrire_quatre_octets(fd, &entete->bitmap.taille_donnees_image);
	ecrire_quatre_octets(fd, &entete->bitmap.resolution_horizontale);
	ecrire_quatre_octets(fd, &entete->bitmap.resolution_verticale);
	ecrire_quatre_octets(fd, &entete->bitmap.taille_palette);
	ecrire_quatre_octets(fd, &entete->bitmap.nombre_de_couleurs_importantes);
	return 0;
}

int calculPadding(entete_bmp *entete){
	uint32 largeur = entete->bitmap.largeur;
	int padding = (largeur*3)%4;
	if(padding != 0){
		padding = 4 - padding;
	}
	return padding;
}

unsigned char* allouer_pixels(entete_bmp *entete){
	uint32 largeur = entete->bitmap.largeur;
	uint32 hauteur = entete->bitmap.hauteur;
	int padding = calculPadding(entete);
	unsigned char* tab = malloc((largeur*3+padding)*hauteur);
	return tab;
}

int lire_pixels(int de, entete_bmp *entete, unsigned char *pixels){
	uint32 largeur = entete->bitmap.largeur;
	uint32 hauteur = entete->bitmap.hauteur;
	int padding = calculPadding(entete);


	int tailleLigne = largeur*3+padding;
	lseek(de,entete->fichier.offset_donnees,SEEK_SET);


	return read(de,pixels,tailleLigne*hauteur);
}

int ecrire_pixels(int vers, entete_bmp *entete, unsigned char *pixels){
	uint32 largeur = entete->bitmap.largeur;
	uint32 hauteur = entete->bitmap.hauteur;
	int padding = calculPadding(entete);
	int tailleLigne = largeur*3+padding;
	lseek(vers,entete->fichier.offset_donnees,SEEK_SET);
	return write(vers,pixels,tailleLigne*hauteur);
}

int main(int argc, char *argv[]){
	int de = open(argv[1],O_RDONLY);
	if(de == -1){
		fprintf(stderr,"fichier inexistant\n");
		return 0;
	}

	if(argc != 4){
		fprintf(stderr,"mauvais nombre d'arguments\n");
		return 0;
	}

	printf("debut\n");
	entete_bmp entete;
	unsigned char *pixels;
	/* lecture du fichier source */

	lire_entete(de, &entete);
	printf("entete lue\n");
	pixels = allouer_pixels(&entete);
	printf("pixels lus\n");
	lire_pixels(de, &entete, pixels);
	int vers = open(argv[2],O_CREAT|O_RDWR|O_TRUNC,S_IRWXU);
	char* text = argv[3];
	printf("%s\n",text);
	entete.fichier.offset_donnees = entete.fichier.offset_donnees + strlen(text);
	

	ecrire_entete(vers, &entete);
	printf("entete ecrite\n");
	//ecrire_texte(vers, text);
	printf("texte ecrit\n");
	ecrire_pixels(vers, &entete, pixels);
	printf("pixels ecrits\n");

	return 1; /* on a réussi */
}

