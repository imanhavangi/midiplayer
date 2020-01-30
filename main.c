#include <stdio.h>
#include <string.h>
#include "beep.h"

//functions:
void getfilename();
void header();
void readevent();
void searchevent(int c1,int c2);
void second_per_time();
void readnote();
int read_delta_time();
void play();
void set_note();
void with(int t);
void without(int t);

//struct:
struct Header_Chunk{
    char Chunk_type[10];
    int length;
    struct Data
    {
        int format;
        int tracks;
        int division;
    }data;
    char MTrk[20];
    int lengthtrack;
}mid;

struct important{
    int settempo;
    int time_0;
    int s_per_t;
    int count;

}important;

struct note_on{
    int deltatime;
    int channel;
	int note;
    char realnote[3];
    float frequency;
	char velocity;
}note_on[2000];
struct note_off{
    int deltatime;
    int channel;
	int note;
	char velocity;
}note_off[2000];

FILE *infile;


int main(){                                         /*****************************************************/
    //forelise.mid
    getfilename();
    header();
    readevent();
    second_per_time();
    readnote();
    set_note();
    play();

}


void getfilename(){
    char namefile[30];
    printf("Enter name of file:\n");
    scanf("%s",namefile);
    infile = fopen(namefile,"r");
    //if(infile == NULL) printf("not found");
}

void header(){
    int i,j,c;
    int sum=0;
    char b[20]; //for read some bytes that not important
    fgets(mid.Chunk_type,5,infile);
    printf("header chunk : %s\n",mid.Chunk_type);
    fgets(b,5,infile);
    mid.length=6;
    printf("length: %d\n",mid.length);

    sum=0;
    for (i = 0; i < 2; i++)
    {
        c=fgetc(infile);
        sum+=c;
        sum*=10;
    }
    mid.data.format=sum/10;
    printf("format of mid file: %d\n",mid.data.format);
    
    sum=0;
    for (i = 0; i < 2; i++)
    {
        c=fgetc(infile);
        sum+=c;
        sum*=10;
    }
    mid.data.tracks=sum/10;
    printf("number of tracks: %d\n",mid.data.tracks);

    sum=0;
    for (i = 0; i < 2 ; i++)
    {
        c=fgetc(infile);
        sum+=c;
        for ( j = 1; j > i; j--)
        {
            sum=sum*16*16;
        }
    }
    mid.data.division=sum;
    printf("division: %d\n",mid.data.division);
    
    fgets(mid.MTrk,5,infile);
    printf("Track header: %s\n",mid.MTrk);

    sum=0;
    for (i = 0; i < 4 ; i++)
    {
        c=fgetc(infile);
        sum+=c;
        for ( j = 3; j > i; j--)
        {
            sum=sum*16*16;
        }
    }
    mid.lengthtrack=sum;
    printf("Track header length: %d dec\n",mid.lengthtrack);
}

void readevent(){
    //printf("rdgs");
    int c0,c1,c2;
    c0=fgetc(infile);
    c1=fgetc(infile);
    //printf("%X",c1);
    while (c1 != 0x90){
        c2=fgetc(infile);
        searchevent(c1,c2);
        c0=fgetc(infile);
        c1=fgetc(infile);
    }
    note_on[0].channel=c1;
    note_on[0].note=fgetc(infile);
    note_on[0].velocity=fgetc(infile);
    note_off[0].deltatime=read_delta_time();
    note_off[0].channel=fgetc(infile);
    note_off[0].note=fgetc(infile);
    note_off[0].velocity=fgetc(infile);
}

void searchevent(int c1,int c2){
    if(c2 == 0x58){
        //printf("s\n");
        printf("data bytes: %d\n",fgetc(infile));
        printf("upper value of time signature : %d\n",fgetc(infile));
        printf("lower value of time signature: %d\n",fgetc(infile));
        printf("number of MIDI clocks in a metronome clock: %d\n",fgetc(infile));
        printf("number of notated 32nd-notes: %d\n",fgetc(infile));
    }
    else if(c2 == 0x51){
        int length,sum;
        important.settempo=0;
        length=fgetc(infile);
        //printf("%d",length);
        for (int i = 0; i < length ; i++)
		{
			sum=fgetc(infile);
			for (int k = length-1; k > i; k--)
			{
				sum=sum*16*16;
			}
			important.settempo+=sum;
		}
        printf("Set Tempo in microseconds per quarter note: %d\n",important.settempo);
    }
    else if(c2 == 0x59){
        int key;
        key=fgetc(infile);
        key=fgetc(infile);
        if(key == 0){
            printf("Key Signature: major key\n");
        }
        else if(key == 1){
            printf("Key Signature: minor key\n");
        }
    }
    else if(c2 == 0x21){
        int n;
        n=fgetc(infile);
        printf("MIDI port : %d\n",fgetc(infile));
    }
    else if(c2 == 0xB0){
        int x;
        x=fgetc(infile);
        x=fgetc(infile);
    }
    else if(c2 == 0xC0){
        int x;
        x=fgetc(infile);
    }
    else if(c2 < 0x90 && c2 >= 0x80){
        int value;
        char delta_time;
        if (value = c2 & 0x80)
        {
            value &=0x7f;
            do
            {
                value = (value << 7) + ((delta_time = fgetc(infile)) & 0x7f);
            } while (delta_time & 0x80);
            
        }
        important.time_0 = value;
    }
}

void second_per_time(){
    double x=60000000/important.settempo;
    double y=x*mid.data.division;
    important.s_per_t=60000/y;
    printf("ticks per a second: %d\n",important.s_per_t);
}

void readnote(){
    //read all notes and print them but only play notes on one channel. user choose this channel.
    int i=1;
    int p;
    while(1){
        note_on[i].deltatime=read_delta_time();
        note_on[i].channel=fgetc(infile);
        if (note_on[i].channel == 0xFF)
        {
            //printf("d");
            break;
        }
        note_on[i].note=fgetc(infile);
        //printf(" k%dk ",note_on[i].note);
        note_on[i].velocity=fgetc(infile);
        note_off[i].deltatime=read_delta_time();
        note_off[i].channel=fgetc(infile);
        note_off[i].note=fgetc(infile);
        note_off[i].velocity=fgetc(infile);
        i++;
        //printf("s");
    }
    important.count=i;
    fclose(infile);
}
int read_delta_time(){
    int value;
    char delta_time;
    if ((value = fgetc(infile)) & 0x80)
    {
        value &=0x7f;
        do
        {
            value = (value << 7) + ((delta_time = fgetc(infile)) & 0x7f);
        } while (delta_time & 0x80);
    }
    return value;
}

void set_note(){
	for (int q = 0; q < important.count; q++)
	{
		if(note_on[q].note%12 == 0 ){
			note_on[q].realnote[0]='C';
			without(q);
		}
		else if(note_on[q].note%12 == 1 ){
			note_on[q].realnote[0]='C';
			note_on[q].realnote[1]='s';
			with(q);
		}
		else if(note_on[q].note%12 == 2 ){
			note_on[q].realnote[0]='D';
			without(q);
		}
		else if(note_on[q].note%12 == 3 ){
			note_on[q].realnote[0]='D';
			note_on[q].realnote[1]='s';
			with(q);
		}
		else if(note_on[q].note%12 == 4 ){
			note_on[q].realnote[0]='E';
			without(q);
		}
		else if(note_on[q].note%12 == 5 ){
			note_on[q].realnote[0]='F';
			without(q);
		}
		else if(note_on[q].note%12 == 6 ){
			note_on[q].realnote[0]='F';
			note_on[q].realnote[1]='s';
			with(q);
		}
		else if(note_on[q].note%12 == 7 ){
			note_on[q].realnote[0]='G';
			without(q);
		}
		else if(note_on[q].note%12 == 8 ){
			note_on[q].realnote[0]='G';
			note_on[q].realnote[1]='s';
			with(q);
		}
		else if(note_on[q].note%12 == 9 ){
			note_on[q].realnote[0]='A';
			without(q);
		}
		else if(note_on[q].note%12 == 10 ){
			note_on[q].realnote[0]='A';
			note_on[q].realnote[1]='s';
			with(q);
		}
		else if(note_on[q].note%12 == 11 ){
			note_on[q].realnote[0]='B';
			without(q);
		}
	}
}
void without(int t){
	if(note_on[t].note<24){
		note_on[t].realnote[1]='0';
	}
	else if(note_on[t].note<36){
		note_on[t].realnote[1]='1';
	}
	else if(note_on[t].note<48){
		note_on[t].realnote[1]='2';
	}
	else if(note_on[t].note<60){
		note_on[t].realnote[1]='3';
	}
	else if(note_on[t].note<72){
		note_on[t].realnote[1]='4';
	}
	else if(note_on[t].note<84){
		note_on[t].realnote[1]='5';
	}
	else if(note_on[t].note<96){
		note_on[t].realnote[1]='6';
	}
	else if(note_on[t].note<108){
		note_on[t].realnote[1]='7';
	}
	else if(note_on[t].note<120){
		note_on[t].realnote[1]='8';
	}
	else if(note_on[t].note<132){
		note_on[t].realnote[1]='9';
	}
}
void with(int t){
	if(note_on[t].note<24){
		note_on[t].realnote[2]='0';
	}
	else if(note_on[t].note<36){
		note_on[t].realnote[2]='1';
	}
	else if(note_on[t].note<48){
		note_on[t].realnote[2]='2';
	}
	else if(note_on[t].note<60){
		note_on[t].realnote[2]='3';
	}
	else if(note_on[t].note<72){
		note_on[t].realnote[2]='4';
	}
	else if(note_on[t].note<84){
		note_on[t].realnote[2]='5';
	}
	else if(note_on[t].note<96){
		note_on[t].realnote[2]='6';
	}
	else if(note_on[t].note<108){
		note_on[t].realnote[2]='7';
	}
	else if(note_on[t].note<120){
		note_on[t].realnote[2]='8';
	}
	else if(note_on[t].note<132){
		note_on[t].realnote[2]='9';
	}
}

void play(){
    FILE *file ;
	//infile=fopen("file.txt","r");
	char *token,read[30];
	for (int q = 0; q < important.count; q++)
	{
		file=fopen("file.txt","r");
		while(fgets (read, 31, file)!=NULL){
			token=strtok(read," ");
			token=strtok(NULL," ");
			if(strcmp(token,note_on[q].realnote)==0){
				token=strtok(NULL," ");
				sscanf(token, "%f", &note_on[q].frequency);
				break; 
			}
		}
		fclose(file);
	}

    beep(0,important.time_0/important.s_per_t);
	for (int q = 0; q < important.count; q++)
	{
		//forelise.mid
		printf("on  %.2f   %d\n",note_on[q].frequency,note_off[q].deltatime/important.s_per_t);
		beep(note_on[q].frequency,note_off[q].deltatime/important.s_per_t*1.7);
        printf("off %.2f   %d\n",note_on[q].frequency,note_on[q+1].deltatime/important.s_per_t);
		beep(0,note_on[q+1].deltatime/important.s_per_t);
        //printf("kggj");
	}
    	printf("\n**end of track**\n");
}
