/* $Id: highscore.c,v 1.2 1998/04/30 05:11:55 mrogre Exp $ */
/* Copyright (c) 1998 Joe Rumsey (mrogre@mediaone.net) */
#include "copyright.h"

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/stat.h>
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#include <ctype.h>
#include <pwd.h>
#include <string.h>
#ifdef _AIX
#include <net/nh.h>
#else /* This is for ntohl, htonl.  
	 This file is correct for hpux and linux for sure, 
	 probably others as well. */
#include <netinet/in.h>
#endif

#include "Wlib.h"
#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "data.h"

/* Global score file is insecure and rather useless today. */
#define NO_GLOBAL_SCORES 1
#ifdef NO_GLOBAL_SCORES
#define NUM_GLOBAL_SCORES 0
#else
#define NUM_GLOBAL_SCORES 20
#endif
#define NUM_MY_SCORES 10

#define MAX_PHONE 12
#define MAX_EMAIL 50
#define MAX_NAME  25


static char new_name[MAX_NAME];
static char new_phone[MAX_PHONE];
static char new_email[MAX_EMAIL];

static int nnpos     = 0;
static int phonepos  = 0;
static int emailpos  = 0;
static int thisplace = -1, my_thisplace = -1;

static struct high_score {
#ifdef NOSCOREHOGS
    long uid;
#endif
    char name[MAX_NAME];
    long score;
    long level;
    char email[MAX_EMAIL];
    char phone[MAX_PHONE];
} 
#ifndef NO_GLOBAL_SCORES
	global_scores[NUM_GLOBAL_SCORES],
#endif
my_scores[NUM_MY_SCORES], cur_score;	

void undo_name() {
    W_ClearArea(baseWin, 
		WINWIDTH/2-(MAX_NAME*W_Textwidth), 250,
		(40*W_Textwidth), W_Textheight*2);
}

void do_name() {
    char buf[21];
    static int init = 0;

    if(!init) {
		strcpy(new_name, getUsersFullName());
		nnpos = strlen(new_name);
		init = 1;
    }
    center_text("Great score! Enter your name:", 250, W_Red);
    sprintf(buf, "%s_", new_name);
    center_text(buf, 250 + W_Textheight, W_Cyan);
}


void do_phone() {
    char buf[MAX_PHONE];
	nnpos = strlen(new_name);
    center_text("enter your phone:", 250, W_Red);
    sprintf(buf, "%s_", new_name);
    center_text(buf, 250 + W_Textheight, W_Cyan);
}

void do_email() {
    char buf[MAX_EMAIL];
	nnpos = strlen(new_name);
    center_text("enter your email:", 250, W_Red);
    sprintf(buf, "%s_", new_name);
    center_text(buf, 250 + W_Textheight, W_Cyan);
}

void do_disclaimer() {
    char buf[MAX_EMAIL];
    nnpos = strlen(new_name);
    center_text("would you like to enter your information to blah? [Y/N]", 250, W_Red);
    sprintf(buf, "%s_", new_name);
    center_text(buf, 250 + W_Textheight, W_Cyan);
}

static void save_scores_new() {
	int i, hsf;
    long x;
    char my_file_name [256], *home;
	if((home = getenv("HOME"))) {
		snprintf(my_file_name, sizeof(my_file_name)-1, "%s/.xgalscores_new", home);
		hsf = open(my_file_name, O_WRONLY | O_TRUNC | O_CREAT, 0644);
		if(hsf < 0) {
		    printf("Couldn't write scores file %s\n", my_file_name);
		    return;
		}
		for(i=0;i<NUM_MY_SCORES;i++) {
		    if(write(hsf, my_scores[i].name, MAX_NAME) < MAX_NAME)
				goto error2;
		    
		    x=htonl(my_scores[i].score);
		    if(write(hsf, &x, sizeof(long)) < sizeof(long))
				goto error2;
		    
		    x=htonl(my_scores[i].level);
		    if(write(hsf, &x, sizeof(long)) < sizeof(long))
				goto error2;
		}
        close(hsf);
    }
	
    return;

	error2:
    	printf("Error saving high scores file %s\n", my_file_name);
    	return;

}

int add_total_highscore(char *name, int score, char *email, char *phone){
	int i,j;
	for(i=0;i<NUM_MY_SCORES;i++) {
		if(cur_score.score > my_scores[i].score) {
		    for(j=NUM_MY_SCORES-1;j>i;j--) {
				strcpy(my_scores[j].name,   my_scores[j-1].name);
				strcpy(my_scores[j].email,  my_scores[j-1].email);
				strcpy(my_scores[j].phone,  my_scores[j-1].phone);
				my_scores[j].score = my_scores[j-1].score;
				my_scores[j].level = my_scores[j-1].level;
		    }

		    strcpy(my_scores[i].name,  cur_score.name);
		    strcpy(my_scores[i].email, cur_score.email);
			strcpy(my_scores[i].phone, cur_score.phone);
		    my_scores[i].score = cur_score.score;
		    my_scores[i].level = level;
		    my_thisplace = i;
		    break;
		}
    }
    save_scores_new();
}


int score_key(W_Event *ev) {
    if(getting_name) {
		switch(ev->key) {
		  	case 13:  //carriage return
			case 10:  //line feed
	      	case 269: //no idea
			    getting_name  = 0;
	//		    getting_email = 1;
			    checking_disc = 1;
			    strncpy(cur_score.name = new_name, MAX_NAME);
			    cur_score.score = score;
			    new_name[0] = '\0';
			    nnpos = 0;
			    pagetimer = 300;
			    W_ClearWindow(baseWin);

		    	break;
			case 8:   //backspace
			case 127: //delete
			case 264: //no idea
			    if(nnpos > 0) {
					nnpos--;
					new_name[nnpos] = 0;
			    }
			    break;
			case 'u'+128:
		    	nnpos = 0;
		    	new_name[nnpos] = 0;
		    	break;
			default:
		    	if(nnpos < 19) {
					new_name[nnpos++] = ev->key;
					new_name[nnpos] = 0;
		    	}
		    	break;
		}
		return 1;
    }
    return 0;
}

int disclaimer_key(W_Event *ev){
	char choice = 0;
	if(checking_disc){
		switch(ev->key){
			case 13:
			case 10:
			case 269:
                choice  = new_name[0];
                if(choice == 'y' || choice == 'Y'){
					getting_phone = 1;
				    checking_disc = 0;
				}else if (choice == 'n' || choice == 'N'){
 					getting_email = 0;
					checking_disc =  0; 
                }
				nnpos = 0;
				new_name[nnpos] = '\0';
				W_ClearWindow(baseWin);
			    break;
			case 8:   //backspace
			case 127: //delete
			case 264: //no idea
				if(nnpos > 0) {
					nnpos--;
					new_name[nnpos] = 0;
				}
				break;
			case 'u'+128:
				nnpos = 0;
				new_name[nnpos] = 0;
				break;
			default:
				if(nnpos < 1){
					new_name[nnpos++] = ev->key;
					new_name[nnpos]   = 0;
				}
				break;
		}
		return 1;
    }
    return 0;
}

int email_key(W_Event *ev){
	if(getting_email){
		switch(ev->key){
		//do stuff
			case 13:
			case 10:
			case 269:
				getting_email = 0;
//				getting_phone = 1;
				strncpy(cur_score.email, new_name, MAX_EMAIL);
				nnpos = 0;
				new_name[nnpos] = '\0';
				add_total_highscore();
				W_ClearWindow(baseWin);
			    break;
			case 8:   //backspace
			case 127: //delete
			case 264: //no idea
				if(nnpos > 0) {
					nnpos--;
					new_name[nnpos] = 0;
				}
				break;
			case 'u'+128:
				nnpos = 0;
				new_name[nnpos] = 0;
				break;
			default:
				if(nnpos < 19){
					new_name[nnpos++] = ev->key;
					new_name[nnpos]   = 0;
				}

				break;
		}

		return 1;
    }
    return 0;
}

int phone_key(W_Event *ev){
	if(getting_phone){
		switch(ev->key){
			//do stuff
			case 13:
			case 10:
			case 269:
				getting_phone = 0;
				getting_email = 1;
				strncpy(cur_score.phone, new_name, MAX_PHONE);
				new_name[0] = '\0';
				nnpos = 0;
				W_ClearWindow(baseWin);
				break;
			case 8:   //backspace
			case 127: //delete
			case 264: //no idea
				if(nnpos > 0) { //TODO: change nnpos
					nnpos--;
					new_name[nnpos] = 0;
				}
				break;
			default:
				//if(48 >= ev->key && ev->key <= 57 && phonepos <= 10){
				if(nnpos < 11){
					new_name[nnpos++] = ev->key;
					new_name[nnpos]   = 0;
				}
				break;
		}
	  	return 1;
	}
	return 0;
}

int check_score(int score)
{
    int i;

    load_scores_new(); /* in case someone else has gotten a high score */
#ifndef NO_GLOBAL_SCORES
    for(i=0;i<NUM_GLOBAL_SCORES;i++) {
	if(score > global_scores[i].score) {
	    return 1;
	}
#ifdef NOSCOREHOGS
 	if(global_scores[i].uid == getuid()) {
	    break;
 	}
#endif
    }
#endif /* NO_GLOBAL_SCORES */
    for(i=0;i<NUM_MY_SCORES;i++) {
	if(score > my_scores[i].score)
	    return 1;
    }

    my_thisplace = -1;
    thisplace = -1;
    return 0;
}


void show_scores()
{
    int i;
    char buf[60];
    W_Color color;

    W_SetRGB16(W_DarkGrey, random()%65535,random()%65535,random()%65535);

    color = W_DarkGrey;

    
#ifndef NO_GLOBAL_SCORES
    center_text("Global high scores", 90, W_Yellow);
    sprintf(buf, "Rank  Name                      Score   Level");
    center_text(buf, 100, W_Yellow);
    W_MakeLine(baseWin, WINWIDTH/2-((strlen(buf)*W_Textwidth)/2), 111,
	       WINWIDTH/2 + ((strlen(buf)*W_Textwidth)/2), 111, W_Red);
    for(i=0;i<NUM_GLOBAL_SCORES;i++) {
	sprintf(buf, "  %2d. %-20s     %7ld %5ld", i+1, 
		global_scores[i].name, global_scores[i].score,global_scores[i].level);
	center_text(buf, 112+i*W_Textheight, (i==thisplace ? color : W_Grey));
    }
#endif
    
    center_text("Your high scores", 112+NUM_GLOBAL_SCORES*W_Textheight, W_Yellow);
    sprintf(buf, "Rank  Name                      Score   Level");
    center_text(buf, 112+(NUM_GLOBAL_SCORES+1)*W_Textheight, W_Yellow);
    W_MakeLine(baseWin, WINWIDTH/2-((strlen(buf)*W_Textwidth)/2), 123+(NUM_GLOBAL_SCORES+1)*W_Textheight,
	       WINWIDTH/2 + ((strlen(buf)*W_Textwidth)/2), 123+(NUM_GLOBAL_SCORES+1)*W_Textheight, W_Red);
    for(i=0;i<NUM_MY_SCORES;i++) {
	sprintf(buf, "  %2d. %-20s     %7ld %5ld", i+1, 
		my_scores[i].name, my_scores[i].score,my_scores[i].level);
	center_text(buf, 124+(NUM_GLOBAL_SCORES+1)*W_Textheight + i*W_Textheight, 
		    (i==my_thisplace ? color : W_Grey));
    }
}

void load_scores_new() {
    int i;
    int hsf;
    char my_file_name[256], *home;

    if((home = getenv("HOME"))) {
		snprintf(my_file_name, sizeof(my_file_name)-1, "%s/.xgalscores_new", home);
		hsf = open(my_file_name, O_RDONLY);
		if(hsf <0 ) {
		    printf("Trouble opening high scores file '%s'\n", my_file_name);
		    for(i=0;i<NUM_MY_SCORES;i++) {
				my_scores[i].name[0]  = 0;
				my_scores[i].email[0] = 0;
				my_scores[i].phone[0] = 0;
				my_scores[i].score = 0;
				my_scores[i].level = 0;
		    }
		} else {
		    for(i=0;i<NUM_MY_SCORES;i++) {
				if(read(hsf, my_scores[i].name, MAX_NAME) < MAX_NAME)
				    goto error2;
				
				if(read(hsf, my_scores[i].email, MAX_EMAIL) < MAX_EMAIL)
				    goto error2;
				
				if(read(hsf, my_scores[i].phone, MAX_PHONE) < MAX_PHONE)
				    goto error2;

				if(read(hsf, &my_scores[i].score, sizeof(long)) < sizeof(long))
				    goto error2;

				if(read(hsf, &my_scores[i].level, sizeof(long)) < sizeof(long))
				    goto error2;

				my_scores[i].score = ntohl(my_scores[i].score);
				my_scores[i].level = ntohl(my_scores[i].level);
		    }
		}
		close(hsf);
    } else {
		printf("No HOME variable, so no personal score file.\n");
		for(i=0;i<NUM_MY_SCORES;i++) {
		    my_scores[i].name[0]  = 0;
		    my_scores[i].email[0] = 0;
		    my_scores[i].phone[0] = 0;
		    my_scores[i].score = 0;
		    my_scores[i].level = 0;
		}
    }
    return;
 
	error2:
	    if(i>0)
		printf("Error reading high scores file '%s'\n", my_file_name);
	    for(i=0;i<NUM_MY_SCORES;i++) {
			my_scores[i].name[0]  = 0;
			my_scores[i].email[0] = 0;
			my_scores[i].phone[0] = 0;
			my_scores[i].score = 0;
			my_scores[i].level = 0;
	    }
    	close(hsf);
}


void print_scores() {
    int i;
    
    load_scores();
#ifndef NO_GLOBAL_SCORES
    printf("\nGlobal High Scores:\n");
    printf("-----------------------------------------------\n");
    printf("%8s %-20s %8s %8s\n", "uid", "name", "score", "level");
    printf("-----------------------------------------------\n");
    for(i=0;i<NUM_GLOBAL_SCORES;i++) {
	if(global_scores[i].score == 0)
	    break;
#ifdef NOSCOREHOGS
	printf("%8ld %-20s %8ld %8ld\n", global_scores[i].uid, 
	       global_scores[i].name, global_scores[i].score, 
	       global_scores[i].level);
#else
	printf("      -- %-20s %8ld %8ld\n",
	       global_scores[i].name, global_scores[i].score,
	       global_scores[i].level);
#endif
    }
#endif /* NO_GLOBAL_SCORES */
    printf("-----------------------------------------------\n");
    printf("\nYour High Scores:\n");
    printf("--------------------------------------\n");
    printf("%-20s %8s %8s\n", "name", "score", "level");
    printf("--------------------------------------\n");
    for(i=0;i<NUM_MY_SCORES;i++) {
	if(my_scores[i].score == 0)
	    break;
	printf("%-20s %8ld %8ld\n", my_scores[i].name, 
	       my_scores[i].score, my_scores[i].level);
    }
    printf("--------------------------------------\n");
}
