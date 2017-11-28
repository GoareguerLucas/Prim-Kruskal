#include "ez-draw.h"
#include <math.h>
#define SIZE 8
#define RANDOM_WGHT 1000
#define DELAY 20 //en ms
#define GROW_SPEED 10//en %

typedef struct dot dot;
typedef struct dotp dotp;
struct dot
{
	int x,y,id;
	dotp *adj;
	dot *next;
	int selected;
	float size_state;
};


struct dotp
{
    int wght;
    dot *dot1;
	dot *dot2;
	dotp *next;
	int mirror;
	int acm;
};


dot *first;
dot *target1, *target2;
int mode,show_wght,dynamic_wght,animation,acm_mode,ultra_mode; //Options
int mx,my,mx2,my2,w,h;
int dots_created,nbr_dot;
int selectx,selecty;
int nbr_dotp;
int ultra1, ultra2, ultraresult;
int GENERATE_VALUE;

void reinit_acm()
{
	dot *current=first;
	dotp *link;
	while(current!=NULL)
	{
		link=current->adj;
		while(link!=NULL)
		{
			link->acm=0;
			link=link->next;
		}
		current=current->next;
	}
}

// Tri par tas d'un tableau de pointeur vers des arretes par rapport a leurs poids

void permute(dotp **tab,int i,int j)
{
    dotp *c;
    c = tab[i];
    tab[i] = tab[j];
    tab[j] = c;
}

int compare_fils(dotp **tab, int n, int taille, int option)
// Renvoi l'indice du fils de n qui a le poid le plus important
{
    if(2*n+2 >= taille) return 2*n+1;
	if(option == 0 && (tab[2*n+1]->dot1->id >= tab[2*n+2]->dot1->id))return 2*n+1;
    if(option == 1 && (tab[2*n+1]->wght >= tab[2*n+2]->wght))return 2*n+1;
    return 2*n+2;
}

void tamiser(dotp **tab, int n, int taille, int option)
{
    int i;

    i = compare_fils(tab, n, taille, option);

    if(option == 1 && (i < taille && tab[i]->wght > tab[n]->wght))
    {
        permute(tab, i, n);
        if(i < taille/2) tamiser(tab, i, taille, option);
    }
    else if(option == 0 && (i < taille && tab[i]->dot1->id > tab[n]->dot1->id))
    {
        permute(tab, i, n);
        if(i < taille/2) tamiser(tab, i, taille, option);
    }
}

void tri_tab_dotp(dotp **tab, int taille, int option)
// Tri le tableau d'arrete selon leurs poids si option = 1 et selon leurs point de depart si option = 0
{
	int i;

	for(i = (taille - 1)/2; i >= 0; i--) tamiser(tab, i, taille, option);

    i = 0;
    while(taille > 0)
    {
        permute(tab, 0, taille-1);
        taille--;
        tamiser(tab, 0, taille, option);
    }
}

// Fin Tri par tas //////////////////////////////////////////////////////////////

int parcour_ACM(dotp **ACM, dotp **pile, int taille_pile, dotp *link, int dot_arriver)
// Parcour l'ACM pour trouver le point d'arriver en sauvegardent le chemin dans le pile
// Retourne la taille de la pile
{
	int j;

	while(link->dot2->id != dot_arriver)
	{
		j = 0;
		while(ACM[j]->dot1->id != link->dot2->id) j++; // J est l'indice de la premiere arrete partant de link->dot2->id dans l'ACM

		do
		{
			if(pile[taille_pile - 1]->dot1->id != ACM[j]->dot2->id)
			{
				pile[taille_pile] = ACM[j];
				taille_pile = parcour_ACM(ACM, pile, taille_pile + 1, ACM[j], dot_arriver);
			}

			if(pile[taille_pile - 1]->dot2->id != dot_arriver)j++;
			else break;
		}while(j < 2 *(nbr_dot - 1) && ACM[j]->dot1->id == link->dot2->id);
		printf("TP : %d \n",taille_pile);

		if(taille_pile == 1) return taille_pile;
		else
		{
			if(pile[taille_pile - 1]->dot2->id != dot_arriver) return taille_pile - 1;
			else break;
		}
	}
	return taille_pile;
}

void explore(dot *current, int passage[])
{
	dotp *link=current->adj;
	passage[current->id]=1;
	while(link!=NULL)
	{
		if(passage[link->dot2->id]!=1) explore(link->dot2,passage);
		link=link->next;
	}
}

int est_connexe()
{
	int passage[nbr_dot];
	for(int i=0;i<nbr_dot;i++)passage[i]=0;
	explore(first,passage);
	for(int i=0;i<nbr_dot;i++) if(passage[i]==0) return 0;
	return 1;
}

int ultrametrique(dotp **ACM, int dot_depart, int dot_arriver)
// Retourne le poids de la plus lourde des arretes composant le chemin de dot_depart a dot_arriver dans l'ACM
{
	dotp *pile[nbr_dot - 1], *ACM_mirror[2 *(nbr_dot - 1)];
	int taille_pile = 1, i, j;
	dotp *link;

	// Remplissage de l'ACM_mirror avec les arretes mirror
	for(i = 0; i < nbr_dot - 1; i++) ACM_mirror[i] = ACM[i];
	for(i = 0, j = nbr_dot - 1; i < nbr_dot - 1; i++, j++)
	{
		link = malloc(sizeof(dotp));

		link->next = ACM_mirror[i]->dot2->adj;
		ACM_mirror[i]->dot2->adj = link;
		link->dot1 = ACM_mirror[i]->dot2;
		link->dot2 = ACM_mirror[i]->dot1;
		link->mirror = ACM_mirror[i]->mirror;
		link->wght = ACM_mirror[i]->wght;

		ACM_mirror[j] = link;
	}

	tri_tab_dotp(ACM_mirror, 2 *(nbr_dot - 1), 0); // Tri de l'AMC_mirror par rapport aux id de dot1 des arretes
	for(i = 0; i < 2*(nbr_dot-1); i++)
	{
		printf("Ult : dot1 : %d dot2 : %d wght : %d\n", ACM_mirror[i]->dot1->id, ACM_mirror[i]->dot2->id, ACM_mirror[i]->wght);
	}printf("\n");
	i = 0;
	while(ACM_mirror[i]->dot1->id != dot_depart) i++; // Initialisation de i (i = index de la premier case de l'AMC où dot_depart est le dot1)

	// Premier case de la pile remplie avec la premiere arrete où dot_depart est le dot1
	link = ACM_mirror[i];
	pile[0] = link;

	while(i < 2 *(nbr_dot - 1) && ACM_mirror[i]->dot1->id == dot_depart) // Pour toutes les arretes partant du points de depart...
	{
		printf("i : %d, TP : %d, dot1 : %d, dot2 : %d, wght : %d, arriver : %d\n", i, taille_pile, ACM_mirror[i]->dot1->id, ACM_mirror[i]->dot2->id, ACM_mirror[i]->wght, dot_arriver);
		taille_pile = parcour_ACM(ACM_mirror, pile, taille_pile, ACM_mirror[i], dot_arriver); // ...on parcours le chemin...
		if(pile[taille_pile - 1]->dot2->id == dot_arriver) break; //...si le chemin mene a dot_arriver on arrete
		i++; // ...sinon on continu
	}

	// On compare les arrete de la pile pour trouver le plus grand
	link = pile[0];
	printf("Taillle P : %d\n",taille_pile);
	for(i = 1; i < taille_pile; i++)
	{
		if(pile[i]->wght > link->wght) link = pile[i];
		printf("Ultrametrique : dot1 : %d dot2 : %d wght : %d\n", pile[i]->dot1->id, pile[i]->dot2->id, pile[i]->wght);
	}
	printf("Ultrametrique : dot1 : %d dot2 : %d wght : %d\n", link->dot1->id, link->dot2->id, link->wght);
	return link->wght;
}

int maj_tas(dotp **tas_dotp, int taille_tas, dot *current, int *dots_vus)
{
	dotp *link = current->adj;

	while(link != NULL)
	{
		if(dots_vus[link->dot2->id] == 0)
		{
			tas_dotp[taille_tas] = link;
			taille_tas++;
		}
		link = link->next;
	}

	tri_tab_dotp(tas_dotp, taille_tas, 1); // Tri en fonction du poids des arretes
	return taille_tas;
}

void Prim()
{
    if(nbr_dotp<=0) return;
	dotp *ACM[nbr_dot - 1], *tas_dotp[nbr_dotp];
	dot *current = first;
	dot *current2 = first;
	dotp *link = current2->adj;
	int taille_tas = 0, nbr_dotp_select = 0, i;
	int dots_vus[nbr_dot];

    reinit_acm();

	current = first;
	link = current2->adj;

	// Initialisation
	for(i = 0; i < nbr_dotp; i++) tas_dotp[i] = NULL;
	for(i = 0; i < nbr_dot; i++) dots_vus[i] = 0;
	dots_vus[0] = 1;

	// Remplissage de L'ACM par parcour des sommets
	i = 0;
	while(nbr_dotp_select < nbr_dot - 1)
	{
		taille_tas = maj_tas(tas_dotp, taille_tas, current, dots_vus);	// Mise a jour du tas

		while(link != NULL)
		{
			while(link != NULL)
			{
				link = link->next;
			}
			current2 = current2->next;
			if(current2 != NULL)link = current2->adj;
		}

		i = 0;
		while(dots_vus[tas_dotp[i]->dot2->id] == 1) i++;	// On passe les arretes qui menne vers des sommets deja visite
		ACM[nbr_dotp_select] = tas_dotp[i]; 	// Ajout de la plus petite arrete dans l'AMC
		nbr_dotp_select++;

		current = tas_dotp[i]->dot2;	// Passage au sommet suivant
		dots_vus[current->id] = 1;		// Sommet marquer comme vu
	}

	// Affichage de L'ACM
	//printf("Prim : \n");
	for(i = 0; i < nbr_dot - 1; i++)
    {
		//printf("i %d ",i);
        //printf("%d %d %d \n",ACM[i]->wght, ACM[i]->dot1->id, ACM[i]->dot2->id);
        ACM[i]->acm = 1;
    }
    
    if(ultra_mode == 1)
		if(ultra1 >= 0 && ultra2 >= 0)
			printf("ultrametrique : %d \n",ultrametrique(ACM, ultra1, ultra2));
}

void Kruskal()
{
    if(nbr_dotp <= 0) return;
	dot *current = first;
	dotp *link;
	dotp *ACM[nbr_dot -1], *tab_dotp[nbr_dotp];
	int i = 0;

	// Remplissage du tableau des arretes
	while(current != NULL)
	{
        link=current->adj;
		while(link != NULL)
		{
            link->acm=0;
			if(link->mirror == 1)
			{
				tab_dotp[i] = link;
				i++;
			}
			link = link->next;
		}
		current = current->next;
	}

	tri_tab_dotp(tab_dotp, nbr_dotp, 1);	// Tri en fonction du poids des arretes

	// Initialisation
	int tab_pere[nbr_dot], tab_hauteur[nbr_dot];
	int j = 0;
	int sommet1, sommet2;
	for(i = 0; i < nbr_dot; i++) {tab_pere[i] = i; tab_hauteur[i] = 1;}
	// Parcours des arretes
	i = 0;
	while(i < nbr_dot - 1)
	{
		for(; j < nbr_dotp; j++)
		{
			sommet1 = tab_dotp[j]->dot1->id;
			sommet2 = tab_dotp[j]->dot2->id;
			while(tab_pere[sommet1] != sommet1) sommet1 = tab_pere[sommet1];
			while(tab_pere[sommet2] != sommet2) sommet2 = tab_pere[sommet2];
			if(sommet1 != sommet2)
			{
				ACM[i] = tab_dotp[j];
				i++;
				tab_pere[sommet2] = tab_pere[sommet1];
				tab_hauteur[sommet1]++;
			}
		}
	}
	// Affichage de L'ACM
	printf("Kruskal : \n");
	for(i = 0; i < nbr_dot - 1; i++)
    {
		printf("i %d ",i);
        printf("%d %d %d \n",ACM[i]->wght, ACM[i]->dot1->id, ACM[i]->dot2->id);
		ACM[i]->acm = 1;
    }
    printf("\n");
	if(ultra_mode == 1)
		if(ultra1 >= 0 && ultra2 >= 0)
			printf("ultrametrique : %d \n",ultrametrique(ACM, ultra1, ultra2));
}

int my_round (float value)
{
    return value + 0.5;
}

int clicked (dot *current)
{
	if(mx<current->x+SIZE && mx>current->x-SIZE && my<current->y+SIZE && my>current->y-SIZE) return 1;
	else return 0;
}

int inside_selection(dot *current)
{
    if(mx>selectx)
    {
        if(my>selecty)
        {
            if(current->x>selectx && current->x<mx && current->y>selecty && current->y<my) return 1;
            else return 0;
        }
        else
        {
            if(current->x>selectx && current->x<mx && current->y<selecty && current->y>my) return 1;
            else return 0;
        }
    }
    else
    {
        if(my>selecty)
        {
            if(current->x<selectx && current->x>mx && current->y>selecty && current->y<my) return 1;
            else return 0;
        }
        else
        {
            if(current->x<selectx && current->x>mx && current->y<selecty && current->y>my) return 1;
            else return 0;
        }
    }
}

void win1_on_expose (Ez_event *ev)
{
	dot *current = first;
	int midx,midy;
	if(est_connexe())
	{
		ez_set_color(ez_green);
		ez_draw_text(ev->win,EZ_BL,0,h-20,"CONNEXE");
	}
	else
	{
		ez_set_color(ez_red);
		ez_draw_text(ev->win,EZ_BL,0,h-20,"NON CONNEXE");
	}

	ez_set_color(ez_red);
	if(acm_mode==0) ez_draw_text(ev->win,EZ_BL,0,h-10,"Prim");
	else ez_draw_text(ev->win,EZ_BL,0,h-10,"Kruskal");

	if(ultra_mode==1)
	{
		ez_set_color(ez_green);
		ez_draw_text(ev->win,EZ_BL,0,h,"ULTRAMETRIE ACTIVE");
	}
	else
	{
		ez_set_color(ez_red);
		ez_draw_text(ev->win,EZ_BL,0,h,"ULTRAMETRIE DESACTIVEE");
	}
    ultraresult=0;
	if(est_connexe())
	{
		if(acm_mode==0) Prim();
		else Kruskal();
	}

	ez_set_color(ez_black);
	ez_draw_text(ev->win,EZ_TL,10,10,"A: Ajouter un sommet     Z: Activer l'affichage des poids     Suppr: Supprimer le sommet selectionne     W: Prim/Kruskal\nE: Generer %d sommets lies (Q/S pour reduire/augmenter)     R: Activer la dynamisation des poids     T: Activer les animations\nX: Activer ultra mode\nSelectionnez/deplacez les sommets avec le clic gauche, liez-les avec le clic droit.",GENERATE_VALUE);
	if(mode==2)
	{
        ez_set_color(ez_grey);
		ez_draw_line(ev->win,target1->x,target1->y,mx,my);
	}
	else if(mode==3)
	{
        ez_set_color(ez_black);
        ez_draw_rectangle(ev->win,selectx,selecty,mx,my);
	}
    while(current != NULL)
    {
		dotp *linked=current->adj;
		while(linked!=NULL)
		{
			if(linked->dot2->id>current->id)
			{
			    ez_set_color(ez_black);
                ez_draw_line(ev->win,current->x,current->y,linked->dot2->x,linked->dot2->y);
                if(show_wght==1)
                {
                    midx = current->x-(current->x-linked->dot2->x)/2;
                    midy = current->y-(current->y-linked->dot2->y)/2;
                    ez_draw_text(ev->win,EZ_BL,midx,midy,"%d",linked->wght);
                }
            }
            if(linked->acm==1)
            {
                ez_set_thick(5);
                ez_set_color(ez_cyan);
                ez_draw_line(ev->win,current->x,current->y,linked->dot2->x,linked->dot2->y);
                ez_set_thick(1);
            }
			linked=linked->next;
		}
		if(current->selected==1) ez_set_color(ez_red);
		else if(inside_selection(current) && mode==3) ez_set_color(ez_green);
		else ez_set_color(ez_blue);
		ez_fill_rectangle(ev->win,current->x+my_round(current->size_state),current->y+my_round(current->size_state),current->x-my_round(current->size_state),current->y-my_round(current->size_state)); //Animation test
		if(ultra_mode==1)
		{
			ez_set_color(ez_red);
			ez_draw_text(ev->win,EZ_BR,w,h,"%d",ultraresult);
			if(current->id == ultra1 || current->id == ultra2)
			{
				ez_set_thick(3);
				ez_draw_circle(ev->win,current->x-SIZE-10,current->y-SIZE-10,current->x+SIZE+10,current->y+SIZE+10);
				ez_set_thick(1);
			}
		}
		current=current->next;
	}
}


void win1_on_button_press (Ez_event *ev)
{
	dot *current = first;
	if(ultra_mode==0)
	{
		if(ev->mb==1)
		{
			while(current!=NULL)
			{
				if(clicked(current))
				{
					mode=1;
					if(current->selected==0)
					{
						target1=current;
						current=first;
						while(current!=NULL)
						{
							current->selected=0;
							current=current->next;
						}
						target1->selected=1;
					}
					break;
				}
				current = current->next;
			}
			if(mode!=1)
			{
				current=first;
				while(current!=NULL)
				{
					current->selected=0;
					current=current->next;
				}
				selectx=mx;
				selecty=my;
				mode=3;
			}
		}
		else if(ev->mb==3)
		{
			while(current!=NULL)
			{
				if(clicked(current))
				{
					mode=2;
					target1=current;
					break;
				}
				current = current->next;
			}
		}
	}
	else
	{
		while(current!=NULL)
		{
			if(clicked(current))
			{
				if(ev->mb==1)
				{
					if(ultra2!=current->id) ultra1=current->id;
				}
				else if(ev->mb==3)
				{
					if(ultra1!=current->id) ultra2=current->id;
				}
				break;
			}
			current = current->next;
		}
	}
	ez_send_expose(ev->win);
}

void un_link(dotp *linked, dot *dot1, dot *dot2)
{
	reinit_acm();
    nbr_dotp--;
	dotp *temp;
	if(linked->dot2==dot2)
	{
        if(linked->next!=NULL)
        {
            dot1->adj=linked->next;
            free(linked);
        }
        else
        {
            dot1->adj=NULL;
            free(linked);
        }
	}
	else
	{
        if(linked->next->next!=NULL)
        {
            temp=linked->next->next;
            free(linked->next);
            linked->next=temp;
        }
        else
        {
            free(linked->next);
            linked->next=NULL;
        }
    }
    linked=dot2->adj;
	if(linked->dot2==dot1)
	{
		if(linked->next!=NULL)
        {
            dot2->adj=linked->next;
            free(linked);
        }
        else
        {
            free(linked);
            dot2->adj=NULL;
        }
		mode=0;
		return;
	}
	while(linked->next!=NULL)
	{
		if(linked->next->dot2==dot1)
		{
			if(linked->next->next!=NULL)
            {
                temp=linked->next->next;
                free(linked->next);
                linked->next=temp;
            }
            else
            {
                free(linked->next);
                linked->next=NULL;
            }
            break;
		}
		linked=linked->next;
	}
	mode=0;
	return;
}

void link(int dotid1, int dotid2, int poids)
{
	reinit_acm();
    dot *current=first;
    target1=NULL;
    target2=NULL;
    /*Pour la simplification de l'appel de link, même si on possède déjà les adresses des targets,
        on les recherchera de nouveau à partir de leur id.*/
    while(current!=NULL && (target1==NULL || target2==NULL))
    {
        if(current->id==dotid1) target1=current;
        else if(current->id==dotid2) target2=current;
        current=current->next;
    }
    dotp *linked=target1->adj;
    dotp *temp;
    if(linked!=NULL)
    {
        if(linked->dot2->id==target2->id) /*Si le lien existe déjà et est le premier de la liste*/
        {
            un_link(linked, target1, target2);
            return;
        }
        while(linked->next!=NULL)
        {
            if(linked->next->dot2->id==target2->id) /*Si le lien existe déjà*/
            {
                un_link(linked, target1, target2);
                return;
            }
            linked=linked->next;
        }

        linked=target1->adj;
        if(linked->wght>poids)
        {
            temp=malloc(sizeof(dotp));
            temp->next=target1->adj;
            target1->adj=temp;
            linked=target1->adj;
            linked->dot1=target1;
            linked->dot2=target2;
            linked->wght=poids;
            linked->mirror=0;
            linked->acm=0;
        }
        else
        {
            while(linked->next!=NULL)
            {
                if(linked->next->wght>poids) break;
                linked=linked->next;
            }
            temp=malloc(sizeof(dotp));
            temp->next=linked->next;
            linked->next=temp;
            linked=linked->next;
            linked->dot1=target1;
            linked->dot2=target2;
            linked->wght=poids;
            linked->mirror=0;
            linked->acm=0;
        }
    }
    else
    {
        target1->adj=malloc(sizeof(dotp));
        linked=target1->adj;
        linked->dot1=target1;
        linked->dot2=target2;
        linked->next=NULL;
        linked->wght=poids;
        linked->mirror=0;
        linked->acm=0;
    }
    nbr_dotp++;

    dotp *mirrorlink=linked->dot2->adj;
    if(mirrorlink!=NULL)
    {
        if(mirrorlink->wght>poids)
        {
            temp=malloc(sizeof(dotp));
            temp->next=target2->adj;
            target2->adj=temp;
            mirrorlink=target2->adj;
            mirrorlink->dot1=target2;
            mirrorlink->dot2=target1;
            mirrorlink->wght=poids;
            mirrorlink->mirror=1;
            mirrorlink->acm=0;
        }
        else
        {
            while(mirrorlink->next!=NULL)
            {
                if(mirrorlink->next->wght>poids) break;
                mirrorlink=mirrorlink->next;
            }
            temp=malloc(sizeof(dotp));
            temp->next=mirrorlink->next;
            mirrorlink->next=temp;
            mirrorlink=mirrorlink->next;
            mirrorlink->dot1=target2;
            mirrorlink->dot2=target1;
            mirrorlink->wght=poids;
            mirrorlink->mirror=1;
            mirrorlink->acm=0;
        }
    }
    else
    {
        target2->adj=malloc(sizeof(dotp));
        mirrorlink=target2->adj;
        mirrorlink->dot1=target2;
        mirrorlink->dot2=target1;
        mirrorlink->next=NULL;
        mirrorlink->wght=poids;
        mirrorlink->mirror=1;
        mirrorlink->acm=0;
    }
}

void win1_on_button_release (Ez_event *ev)          /* Mouse button released */
{
	if(ultra_mode==0)
	{
		if(ev->mb==1)
		{
			dot *current=first;
			if(mode==3)
			{
				while(current!=NULL)
				{
					if(inside_selection(current)) current->selected=1;
					current=current->next;
				}
			}
			mode=0;
		}
		else if(ev->mb==3)
		{
			dot *current = first;
			while(current!=NULL)
			{
				if(clicked(current) && target1->id != current->id) /*Si le lien a été relaché sur un autre point*/
				{
					link(target1->id,current->id,ez_random(RANDOM_WGHT));
					ez_send_expose(ev->win);
					break;
				}
				current = current->next;
			}
			mode=0;
		}
		ez_send_expose(ev->win);
	}
}


void win1_on_motion_notify (Ez_event *ev)                     /* Mouse moved */
{
    mx2=mx;
    my2=my;
	mx=ev->mx;
	my=ev->my;
	if(mode==1)
	{
        dot *current=first;
		while(current!=NULL)
		{
            if(current->selected==1)
            {
                current->x+=mx-mx2;
                current->y+=my-my2;
                if(dynamic_wght==1)
                {
					int distance;
					dotp *linked=current->adj;
                    dotp *linktrace=linked;
					dotp *mirrorlink;
					dotp *mirrortrace;
					dotp *temp;
					while(linked!=NULL)
					{
						distance = sqrt(pow(current->x-linked->dot2->x,2)+pow(current->y-linked->dot2->y,2));
						linked->wght=distance;

						if(linked->next!=NULL)
                            if(linked->next->wght<linked->wght)
                            {
                                temp = linked->next;
                                linked->next = temp->next;
                                temp->next = linked;
                                if(linktrace==linked) current->adj=temp;
                                else linktrace->next=temp;
                            }

						mirrorlink=linked->dot2->adj;
                        mirrortrace=mirrorlink;
						while(mirrorlink->dot2->id!=current->id)
						{
                            mirrortrace=mirrorlink;
							mirrorlink=mirrorlink->next;
						}
						if(mirrorlink->next!=NULL)
                            if(mirrorlink->next->wght<mirrorlink->wght)
                            {
                                temp = mirrorlink->next;
                                mirrorlink->next = temp->next;
                                temp->next = mirrorlink;
                                if(mirrortrace==mirrorlink) linked->dot2->adj=temp;
                                else mirrortrace->next=temp;
                            }
						mirrorlink->wght = distance;
						linktrace=linked;
						linked=linked->next;
					}
				}
            }
            current=current->next;
		}
	}
	if(mode!=0) ez_send_expose(ev->win);
}

dot* create_dot()
//Retourne l'id du sommet créé
{
	reinit_acm();
    nbr_dot++;
    dot *current=first;
    while(current->next!=NULL)
    {
        current = current->next;
    }
    current->next=malloc(sizeof(dot));
    current=current->next;
    current->x=ez_random(w);
    current->y=ez_random(h);
    current->adj=NULL;
    current->next=NULL;
    current->id=nbr_dot-1;
    current->selected=0;
    if(animation==1) current->size_state=1;//Test d'animation
    else current->size_state=SIZE;
    return current;
}

void dot_cloud_generation(int nbr)
{
    int i,distance;
    dot *current, *created_dot;
    for(i=0;i<nbr;i++)
    {
        created_dot = create_dot();
        current = first;
        while(current->id!=created_dot->id)
        {
            distance = sqrt(pow(current->x-created_dot->x,2)+pow(current->y-created_dot->y,2));
            link(created_dot->id, current->id,distance);
            current=current->next;
        }
    }
}

void win1_on_key_press (Ez_event *ev)                         /* Key pressed */
{
    if(ev->key_sym==XK_z)
    {
        if(show_wght==0) show_wght=1;
        else show_wght=0;
    }
    else if(ev->key_sym==XK_a)
    {
        ez_window_get_size(ev->win,&w,&h);
		create_dot();
	}
	else if(ev->key_sym==XK_e)
    {
        ez_window_get_size(ev->win,&w,&h);
        dot_cloud_generation(GENERATE_VALUE);
        ez_send_expose(ev->win);
	}
	else if(ev->key_sym==XK_r)
    {
        if(dynamic_wght==0) dynamic_wght=1;
        else dynamic_wght=0;
	}
	else if(ev->key_sym==XK_t)
    {
        if(animation==0) animation=1;
        else animation=0;
	}
	else if(ev->key_sym==XK_s)
    {
        GENERATE_VALUE++;
	}
	else if(ev->key_sym==XK_q)
    {
        GENERATE_VALUE--;
	}
	else if(ev->key_sym==XK_w)
    {
        if(acm_mode==1) acm_mode=0;
        else acm_mode=1;
	}
	else if(ev->key_sym==XK_x)
    {
        if(ultra_mode==1) ultra_mode=0;
        else ultra_mode=1;
	}
	else if(ev->key_sym==XK_Delete)
    {
        dot *current=first;
        dot *temp;
		if(current->next==NULL) return;
		while(current->selected==1)
		{
            if(current->next==NULL) return;
            while(current->adj!=NULL)
            {
                un_link(current->adj, current, current->adj->dot2);
            }
            first = current->next;
            free(current);
            nbr_dot--;
            current=first;
		}
        while(current->next!=NULL)
        {
            if(current->next->selected==1)
            {
                while(current->next->adj!=NULL)
                {
                    un_link(current->next->adj, current->next, current->next->adj->dot2);
                }
                temp=current->next;
                current->next=current->next->next;
                free(temp);
                nbr_dot--;
            }
            else current=current->next;
        }
        current=first;
        current->id=0;
        while(current->next!=NULL)
        {
            current->next->id=current->id+1;
            current=current->next;
        }
	}
	ez_send_expose(ev->win);
}


void win1_on_configure_notify (Ez_event *ev)
{
	ez_window_get_size(ev->win,&w,&h);
    ez_send_expose(ev->win);
}

void win1_on_timer_notify (Ez_event *ev)
{
    ez_start_timer(ev->win, DELAY);
    if(animation==1)
    {
        dot *current=first;
        int animated=0;
        while(current!=NULL)
        {
            if(current->size_state!=SIZE)
            {
                current->size_state+=(current->size_state*GROW_SPEED)/100;
                if(current->size_state>SIZE) current->size_state = SIZE;
                animated=1;
            }
            current=current->next;
        }
        if(animated==1) ez_send_expose(ev->win);
    }
}


void win1_on_event (Ez_event *ev)                /* Called by ez_main_loop() */
{                                                /* for each event on win1   */
    switch (ev->type) {
        case Expose          : win1_on_expose           (ev); break;
        case ButtonPress     : win1_on_button_press     (ev); break;
        case ButtonRelease   : win1_on_button_release   (ev); break;
        case MotionNotify    : win1_on_motion_notify    (ev); break;
        case KeyPress        : win1_on_key_press        (ev); break;
        case ConfigureNotify : win1_on_configure_notify (ev); break;
        case TimerNotify	 : win1_on_timer_notify		(ev); break;
   }
}


int main ()
{
	mode=0;
	show_wght=0;
	dynamic_wght=0;
	animation=1;
	GENERATE_VALUE=5;
	acm_mode=1;
	ultra_mode=0;
	ultra1=-1;
	ultra2=-2;
	w=800;
	h=600;

    if (ez_init() < 0) exit(1);

    Ez_window win1 = ez_window_create (w, h, "Graph test", win1_on_event);

    ez_window_dbuf(win1,1);
    nbr_dot=0;
    nbr_dotp=0;
    dots_created=-1;
	first = malloc(sizeof(dot));
	dots_created++;
	nbr_dot++;
	first->x=ez_random(800);
	first->y=ez_random(600);
	first->adj=NULL;
	first->next=NULL;
	first->id=dots_created;
	first->selected=0;
	first->size_state=1;

    ez_start_timer(win1, DELAY);

    ez_main_loop ();
    exit(0);
}

