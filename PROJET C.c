#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LONGUEUR_PISTE 20
#define LARGEUR_PISTE 10
#define VITESSE_PAS_LONG 1
#define VITESSE_PAS_LAT 1
#define DUREE_CYCLE_BASE 0.5
#define PROBABILITE_OBSTACLE 5
#define NORMALE "NORMALE"
#define URGENTE "URGENTE"
#define AVANCER "AVANCER"
#define RECULER "RECULER"
#define TOURNER_GAUCHE "TOURNER_GAUCHE"
#define TOURNER_DROITE "TOURNER_DROITE"
typedef struct {
    char type[30];
    float duree;
    float vitesse;
    char priorite[10];
} Commande;
typedef struct Noeud {
    Commande cmd;
    struct Noeud *suivant;
} Noeud;
typedef struct {
    Noeud *tete;
    Noeud *queue;
} File;
int robot_longitudinal_pos = 0;
int robot_lateral_pos = 0;
float temps_simulation = 0;
File file_normale;
File file_urgente;
int obstacle_present = 0;
int obstacle_x = -1;
int obstacle_y = -1;
void init_file(File *f) { f->tete = f->queue = NULL; }
int file_vide(File *f) { return f->tete == NULL; }
void enfiler(File *f, Commande c) {
    Noeud *n = (Noeud*)malloc(sizeof(Noeud));
    n->cmd = c; n->suivant = NULL;
    if(file_vide(f)) f->tete = f->queue = n;
    else { f->queue->suivant = n; f->queue = n; }
}
Commande defiler(File *f) {
    Noeud *n = f->tete; Commande c = n->cmd;
    f->tete = n->suivant; if(f->tete==NULL) f->queue=NULL;
    free(n); return c;
}
void journaliser(const Commande *c, const char *action) {
    FILE *f = fopen("journal_robot.txt","a"); if(!f) return;
    fprintf(f,"[%.2fs] %s | %s | %.1fs | %.1f | %s\n",
        temps_simulation, c->priorite, c->type, c->duree, c->vitesse, action);
    fprintf(f,"POSITION : x=%d y=%d\n",robot_longitudinal_pos, robot_lateral_pos);
    fclose(f);
}
int capteur_detecte_obstacle() {
    int pseudo_alea = ((int)(temps_simulation*10)) % PROBABILITE_OBSTACLE;
    if(robot_longitudinal_pos>LONGUEUR_PISTE/3 &&
       robot_longitudinal_pos<(LONGUEUR_PISTE*2)/3 &&
       pseudo_alea==0) {
        obstacle_present = 1;
        obstacle_x = robot_longitudinal_pos + 2;
        if(obstacle_x >= LONGUEUR_PISTE) obstacle_x = LONGUEUR_PISTE-1;
        obstacle_y = robot_lateral_pos;
        return 1;
    }
    obstacle_present = 0;
    return 0;
}
void afficher_piste() {
    printf("\n");
    for(int y=-1;y<=LARGEUR_PISTE;y++){
        for(int x=-1;x<=LONGUEUR_PISTE;x++){
            if(x==-1 || x==LONGUEUR_PISTE || y==-1 || y==LARGEUR_PISTE) printf("#");
            else if(x==robot_longitudinal_pos && y==robot_lateral_pos) printf("R");
            else if(obstacle_present && x==obstacle_x && y==obstacle_y) printf("O");
            else printf("_");
        }
        printf("\n");
    }
    printf("\n");
}
void executer_commande(Commande c) {
    if(strcmp(c.priorite,URGENTE)==0){
        printf("!!! COMMANDE URGENTE : %s\n",c.type);
        temps_simulation += DUREE_CYCLE_BASE;
        journaliser(&c,"EXECUTION URGENTE");
        afficher_piste();
        return;
    }
    printf("Commande normale : %s\n",c.type);
    int pas_long=0,pas_lat=0;
    if(strcmp(c.type,AVANCER)==0) pas_long=(int)(c.duree*c.vitesse/VITESSE_PAS_LONG);
    else if(strcmp(c.type,RECULER)==0) pas_long=-(int)(c.duree*c.vitesse/VITESSE_PAS_LONG);
    else if(strcmp(c.type,TOURNER_GAUCHE)==0) pas_lat=-(int)(c.duree*c.vitesse/VITESSE_PAS_LAT);
    else if(strcmp(c.type,TOURNER_DROITE)==0) pas_lat=(int)(c.duree*c.vitesse/VITESSE_PAS_LAT);
    robot_longitudinal_pos += pas_long; robot_lateral_pos += pas_lat;
    if(robot_longitudinal_pos<0) robot_longitudinal_pos=0;
    if(robot_longitudinal_pos>=LONGUEUR_PISTE) robot_longitudinal_pos=LONGUEUR_PISTE-1;
    if(robot_lateral_pos<0) robot_lateral_pos=0;
    if(robot_lateral_pos>=LARGEUR_PISTE) robot_lateral_pos=LARGEUR_PISTE-1;
    temps_simulation += c.duree;
    journaliser(&c,"EXECUTION NORMALE");
    afficher_piste();
}
void gerer_obstacle() {
    obstacle_present = 0;
    Commande c;
    strcpy(c.type,TOURNER_DROITE); c.duree=1; c.vitesse=1; strcpy(c.priorite,NORMALE); enfiler(&file_normale,c);
    strcpy(c.type,AVANCER); c.duree=2; c.vitesse=1; strcpy(c.priorite,NORMALE); enfiler(&file_normale,c);
    strcpy(c.type,TOURNER_GAUCHE); c.duree=1; c.vitesse=1; strcpy(c.priorite,NORMALE); enfiler(&file_normale,c);
    strcpy(c.type,AVANCER); c.duree=1; c.vitesse=1; strcpy(c.priorite,NORMALE); enfiler(&file_normale,c);
    printf("*** Contournement de l'obstacle ajoute ***\n");
}
void ajouter_commandes_initiales(){
    Commande c;
    strcpy(c.type,AVANCER); c.duree=5; c.vitesse=1; strcpy(c.priorite,NORMALE); enfiler(&file_normale,c);
    strcpy(c.type,TOURNER_DROITE); c.duree=3; c.vitesse=1; strcpy(c.priorite,NORMALE); enfiler(&file_normale,c);
    strcpy(c.type,AVANCER); c.duree=4; c.vitesse=1; strcpy(c.priorite,NORMALE); enfiler(&file_normale,c);
    strcpy(c.type,TOURNER_GAUCHE); c.duree=2; c.vitesse=1; strcpy(c.priorite,NORMALE); enfiler(&file_normale,c);
}


void ajouter_commande_normale(Commande c){ enfiler(&file_normale,c); }
void ajouter_commande_urgente(Commande c){ enfiler(&file_urgente,c); }
int prendre_prochaine_commande(Commande *out){
    if(!file_vide(&file_urgente)){ *out=defiler(&file_urgente); return 1; }
    if(!file_vide(&file_normale)){ *out=defiler(&file_normale); return 1; }
    return 0;
}
void executer_commande(Commande c){
    if(strcmp(c.priorite,URGENTE)==0){
        printf("COMMANDE URGENTE : %s\n",c.type);
        temps_simulation+=DUREE_CYCLE_BASE;
        journaliser(&c,"EXECUTION URGENTE");
        afficher_interface();
        return;
    }}


int main(){
    init_file(&file_normale); init_file(&file_urgente);
    FILE *f=fopen("journal_robot.txt","w"); if(f){fprintf(f,"======= Journal Robot =======\n"); fclose(f);}
    float duree_avancer = 5, vitesse_avancer = 1;
    float duree_tourner = 2, vitesse_tourner = 1;
    ajouter_commandes_initiales(duree_avancer,vitesse_avancer,duree_tourner,vitesse_tourner);
    printf("=== Simulation Robot Début ===\n");
    const int MAX_CYCLES=50; int cycle=0;
    Commande c_exec;
    while((!file_vide(&file_normale)||!file_vide(&file_urgente)) && cycle<MAX_CYCLES){
        if(capteur_detecte_obstacle()){
            printf("*** OBSTACLE DETECTE ***\n");
            gerer_obstacle(duree_tourner,vitesse_tourner,duree_avancer,vitesse_avancer);
        }

        if(prendre_prochaine_commande(&c_exec)){
            executer_commande(c_exec);
        }
        cycle++;
    }

    if(cycle>=MAX_CYCLES) printf("=== Simulation arrêtée après %d cycles ===\n",MAX_CYCLES);
    else printf("=== Simulation Robot Terminé ===\n");

    return 0;
}
