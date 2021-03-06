#include <stdio.h>
#include <string.h>
#include "sintable.h"
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
int hex2(int a,int b);
int hex(int a);

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

struct note{
    int deltatime;
    int channel;
	int note;
    char realnote[3];
    float frequency;
	char velocity;
}note[10000];
/*struct note{
    int deltatime;
    int channel;
	int note;
	char velocity;
}note[2000];*/

FILE *infile;

int main(){                                         /*****************************************************/
    //forelise.mid
    important.settempo=500000;
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
    c2=fgetc(infile);
    //printf("%X",c1);
    while (c1 < 0x90 || c1 > 0x99){        
        if(c2 >= 0x90 && c2 <= 0x99){
            break;
        }
        //printf(" %d ",c2);
        searchevent(c1,c2);
        c0=fgetc(infile);
        c1=fgetc(infile);
        c2=fgetc(infile);
        //printf(" %d ",c1);
    }
    // 00 90 2D
    // 81 67 90 2D
    // 00 13 90 2D
    if(c1 >= 0x90){
        note[0].deltatime=0;
        note[0].channel=c1;
        note[0].note=c2;
        note[0].velocity=fgetc(infile);
    }
    else if(c0 >= 0x80){
        note[0].deltatime=hex2(c0,c1)/important.s_per_t;
        printf("sssssssssssss%d",note[0].deltatime);
        note[0].channel=c2;
        note[0].note=fgetc(infile);
        note[0].velocity=fgetc(infile);
    }
    else{
        note[0].deltatime=hex(c1)/important.s_per_t;
        note[0].channel=c2;
        note[0].note=fgetc(infile);
        note[0].velocity=fgetc(infile);
    }
    /*note[0].channel=c1;
    note[0].note=fgetc(infile);
    note[0].velocity=fgetc(infile);
    note[0].deltatime=read_delta_time();
    note[0].channel=fgetc(infile);
    note[0].note=fgetc(infile);
    note[0].velocity=fgetc(infile);*/
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
        //important.settempo=0;
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
    else if(c2 == 0x03){
        int n;
        n=fgetc(infile);
        printf("Track name: ");
        for (int i = 0; i < n; i++)
        {
            printf("%c",fgetc(infile));
        }
        printf("\n");
    }
    else if(c1 == 0x0B){
        int x;
        x=fgetc(infile);
        //x=fgetc(infile);
    }
    else if(c1 == 0xC0){
        //int x;
        //x=fgetc(infile);
    }
    else if(c1 == 0xC2){
        //int x;
        //x=fgetc(infile);
    }
    else if(c1 == 0xB0){
        int x;
        x=fgetc(infile);
    }
    /*else if(c2 < 0x90 && c2 >= 0x80){
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
    }*/
}

void second_per_time(){
    //printf(" %d ",important.settempo);
    //printf("\nholyshit\n");
    double x=60000000/important.settempo;
    double y=x*mid.data.division;
    important.s_per_t=60000/y;
    printf("ticks per a second: %d\n",important.s_per_t);
}

void readnote(){
    //read all notes and print them but only play notes on one channel. user choose this channel.
    int i=1;
    int p,q,w,sum;
    while(1){
        printf("%d ",i);
        p=fgetc(infile);
        q=fgetc(infile);
        if (q == 0xFF )
        {
            p=fgetc(infile);
            if(p == 0x2F){
                printf("\nend of file\n");
                break;
            }
            else if(p == 0x51){
                printf("s00s");
                i--;
                w=fgetc(infile);
                important.settempo=0;
                sum=0;
                for (int j = 0; j < w ; j++)
                {  
                    sum=fgetc(infile);
                    for (int k = w-1; k > j; k--)
                    {
                        sum=sum*16*16;
                    }
                    important.settempo+=sum;
                }
            }
            
        }
        else if(q >= 0x80){
            if(q < 0x90){
                note[i].deltatime=hex(p)/important.s_per_t;
                note[i].channel=q;
                note[i].note=fgetc(infile);
                note[i].velocity=fgetc(infile);
            }else
            {
                note[i].deltatime=hex(p)/important.s_per_t;
                note[i].channel=q;
                note[i].note=fgetc(infile);
                note[i].velocity=fgetc(infile);
            }
        }else
        {
            w=fgetc(infile);
            if (w <= 0x99)
            {
                note[i].deltatime=hex2(p,q)/important.s_per_t;
                note[i].channel=w;
                note[i].note=fgetc(infile);
                note[i].velocity=fgetc(infile);
            }else
            {
                w=fgetc(infile);
                important.settempo=0;
                sum=0;
                for (int j = 0; j < w ; j++)
                {  
                    sum=fgetc(infile);
                    for (int k = w-1; k > j; k--)
                    {
                        sum=sum*16*16;
                    }
                    important.settempo+=sum;
                }
                i--;
            }
            
        }
        i++;

        
    }
    important.count=i;
    fclose(infile);
}
int read_delta_time(){
    /*int value=0;
    char delta_time;
    if ((value = fgetc(infile)) & 0x80)
    {
        value &=0x7f;
        do
        {
            value = (value << 7) + ((delta_time = fgetc(infile)) & 0x7f); 
        } while (delta_time & 0x80);
    }
    return value;*/
    int value;
    value = fgetc(infile) & 0x80;
    if(value == 0){
        
    }
}

int hex(int a){
    int value;
    value = a & 0x80;
    if(value != 0){
        a-=value;
    }
    return a;
}

int hex2(int a,int b){
    int a_temp = a & 0x01;
    int hex_one[4];
    if(a_temp == 0){
        b = b & 0x80;
    }else{
        b = b | 0x80;
    }
    hex_one[0] = b & 0x0F;
    hex_one[1] = b>>4;
    hex_one[2] = (a >> 1) & 0x0F;
    hex_one[3] = (a>>5) & 0x03;

    return hex_one[0] + hex_one[1]*16 + hex_one[2]*256 + hex_one[3]*256*16;

}

void set_note(){
	for (int q = 0; q < important.count; q++)
	{
		if(note[q].note%12 == 0 ){
			note[q].realnote[0]='C';
			without(q);
		}
		else if(note[q].note%12 == 1 ){
			note[q].realnote[0]='C';
			note[q].realnote[1]='s';
			with(q);
		}
		else if(note[q].note%12 == 2 ){
			note[q].realnote[0]='D';
			without(q);
		}
		else if(note[q].note%12 == 3 ){
			note[q].realnote[0]='D';
			note[q].realnote[1]='s';
			with(q);
		}
		else if(note[q].note%12 == 4 ){
			note[q].realnote[0]='E';
			without(q);
		}
		else if(note[q].note%12 == 5 ){
			note[q].realnote[0]='F';
			without(q);
		}
		else if(note[q].note%12 == 6 ){
			note[q].realnote[0]='F';
			note[q].realnote[1]='s';
			with(q);
		}
		else if(note[q].note%12 == 7 ){
			note[q].realnote[0]='G';
			without(q);
		}
		else if(note[q].note%12 == 8 ){
			note[q].realnote[0]='G';
			note[q].realnote[1]='s';
			with(q);
		}
		else if(note[q].note%12 == 9 ){
			note[q].realnote[0]='A';
			without(q);
		}
		else if(note[q].note%12 == 10 ){
			note[q].realnote[0]='A';
			note[q].realnote[1]='s';
			with(q);
		}
		else if(note[q].note%12 == 11 ){
			note[q].realnote[0]='B';
			without(q);
		}
	}
}
void without(int t){
	if(note[t].note<24){
		note[t].realnote[1]='0';
	}
	else if(note[t].note<36){
		note[t].realnote[1]='1';
	}
	else if(note[t].note<48){
		note[t].realnote[1]='2';
	}
	else if(note[t].note<60){
		note[t].realnote[1]='3';
	}
	else if(note[t].note<72){
		note[t].realnote[1]='4';
	}
	else if(note[t].note<84){
		note[t].realnote[1]='5';
	}
	else if(note[t].note<96){
		note[t].realnote[1]='6';
	}
	else if(note[t].note<108){
		note[t].realnote[1]='7';
	}
	else if(note[t].note<120){
		note[t].realnote[1]='8';
	}
	else if(note[t].note<132){
		note[t].realnote[1]='9';
	}
}
void with(int t){
	if(note[t].note<24){
		note[t].realnote[2]='0';
	}
	else if(note[t].note<36){
		note[t].realnote[2]='1';
	}
	else if(note[t].note<48){
		note[t].realnote[2]='2';
	}
	else if(note[t].note<60){
		note[t].realnote[2]='3';
	}
	else if(note[t].note<72){
		note[t].realnote[2]='4';
	}
	else if(note[t].note<84){
		note[t].realnote[2]='5';
	}
	else if(note[t].note<96){
		note[t].realnote[2]='6';
	}
	else if(note[t].note<108){
		note[t].realnote[2]='7';
	}
	else if(note[t].note<120){
		note[t].realnote[2]='8';
	}
	else if(note[t].note<132){
		note[t].realnote[2]='9';
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
			if(strcmp(token,note[q].realnote)==0){
				token=strtok(NULL," ");
				sscanf(token, "%f", &note[q].frequency);
				break; 
			}
		}
		fclose(file);
	}
    for (int i = 0; i < important.count; i++)
    {
        printf("%X  %.2f   %d\n",note[i].channel,note[i].frequency,note[i].deltatime);
    }
    int channel,sum,set;
    //printf("sss");
    //printf("Enter a channel to play notes:\n");
    //scanf("%d",&channel);
    //printf("%d",note[0].deltatime);
    //beep(0,1000);
    //printf("s");
    for (int i = 0; i < important.count; i++)
    {
        //printf("s");
        if(note[i].channel == 0x92){
            sum=0;
            set=i;
            i++;
            while(note[i].channel != 0x82 || note[i].note != note[set].note){
                sum+=note[i].deltatime;
                i++;
            }
            sum+=note[i].deltatime;
            beep(note[set].frequency,sum*4);
            //printf("s");
        }
    }
    
    /*beep(0,important.time_0/important.s_per_t);
	for (int q = 0; q < important.count; q++)
	{
		//forelise.mid
		printf("on  %.2f   %d\n",note[q].frequency,note[q].deltatime/important.s_per_t);
		beep(note[q].frequency,note[q].deltatime/important.s_per_t*1.65);
        printf("off %.2f   %d\n",note[q].frequency,note[q+1].deltatime/important.s_per_t);
		beep(0,note[q+1].deltatime/important.s_per_t);
        //printf("kggj");
        //printf("\n%d\n",q);
	}
    printf("\n**end of track**\n");*/
}
