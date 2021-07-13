#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

int t, k, a, e, fly, count_s = 0, count_m = 0, timer, done_students = 0, t1, t2, min_time, cord[100]={0};
sem_t mutex, mute[100], muter, muts;
//pthread_mutex_t muter;
//pthread_cond_t cond;
pthread_t cor_thread;
pthread_mutex_t tex[100];

typedef struct performer{
    pthread_t performer_thread;
    int id;
    char ins;
    char name[100];
    int time;
    int status;
    char stage;
    char stage_id;
    int active_time;
    int arrival;
    int mute_id;
} Performer;

typedef struct a_stage{
    pthread_t a_thread;
    int id;
    int music;
    int music_id;
    int sing;
    int sing_id;
} a_stage;

typedef struct e_stage
{
    pthread_t e_thread;
    int id;
    int music;
    int music_id;
    int sing;
    int sing_id;
} e_stage;

typedef struct Cord
{
    pthread_t cord_thread;
    int id;
    int status;
} Cord;

Performer **performers;
a_stage **astages;
e_stage **estages;
Cord **cords;

Performer *performer_initialize(int id, Performer* performer)
{
    performer = (Performer *)malloc(sizeof(Performer));
    performer->id = id;
    performer->active_time = 0;
    performer->status = 0;
    performer->arrival = 0;
    performer->mute_id = -1;
    return performer;
}

a_stage *astage_initialize(int id, a_stage* astage)
{
    astage = (a_stage *)malloc(sizeof(a_stage));
    astage->id = id;
    astage->music = 0;
    astage->music_id = -1;
    astage->sing = 0;
    astage->sing_id = -1;
    return astage;
}

e_stage *estage_initialize(int id, e_stage *estage)
{
    estage = (e_stage *)malloc(sizeof(e_stage));
    estage->id = id;
    estage->music = 0;
    estage->sing = 0;
    return estage;
}

Cord *cord_initialize(int id, Cord *cord)
{
    cord = (Cord *)malloc(sizeof(cord));
    cord->id = id;
    cord->status;
    return cord;
}

void *tshirt(void* args)
{
    Cord *cord = (Cord *)args;
    //sem_wait(&mute[cord->id]);
    cords[cord->id]->status = 1;
    int stop = min_time;
    //printf("S\n");
    while(min_time < stop + 2)
    ;
    sem_wait(&muts);
    done_students++;
    sem_post(&muts);
    //printf("done_student: %d\n", done_students);
    cords[cord->id]->status = 0;
    //sem_post(&mute[cord->id]);
}

void *thread_imp(void *args)
{
    Performer *performer = (Performer *)args;
    int random;
    sem_wait(&mutex);
    if (t1 != t2)
        random = rand() % (t2 - t1) + t1 + 1;
    else
        random = t1;
    int stop = min_time;
    sem_post(&mutex);

    if(performer->stage=='a')
    {
        if(astages[performer->stage_id]->music==0)
            printf("%s performing %c solo at acoustic stage %d for %d sec.\n", performer->name, performer->ins, performer->stage_id, random);
        else if (astages[performer->stage_id]->music == 1)
            printf("%s joined %s's performance, performance extended by 2 sec.\n", performer->name, performers[astages[performer->stage_id]->music_id]->name);
    }
    if (performer->stage == 'e')
    {
        if (estages[performer->stage_id]->music == 0)
            printf("%s performing %c solo at acoustic stage for %d sec.\n", performer->name, performer->ins, random);
        if (estages[performer->stage_id]->music == 1)
            printf("%s joined %s's performance, performance extended by 2 sec.\n", performer->name, performers[estages[performer->stage_id]->music_id]->name);
            
    }

    while(min_time < stop + random)
        ;
    if (performer->stage == 'a')
        printf("%s performance at acoustic stage ended.\n", performer->name);
    else
        printf("%s performance at electric stage ended.\n", performer->name);

    sem_wait(&mutex);
    done_students++;
    if (performer->stage == 'a')
    {
        astages[performer->stage_id]->sing = 0;
        astages[performer->stage_id]->sing_id = -1;
    }
    else if(performer->stage=='e')
    {
        estages[performer->stage_id]->sing = 0;
        estages[performer->stage_id]->sing_id = -1;
    }
    
    sem_post(&mutex);
}


void *coord(void *args)
{
    Performer *performer = (Performer *)args;
    while (performers[performer->id]->mute_id == -1)
    {
        for (int i = 0; i < fly; i++)
        {
            //printf("cord %d\n", cord[i]);
            if (cords[i]->status == 0)
            {
                //printf("cord %d\n", cord[i]);
                printf("%s collecting t-shirt.\n", performer->name);
                sem_wait(&muts);
                performers[performer->id]->mute_id = i;
                sem_post(&muts);
                int k = pthread_create(&(cords[i]->cord_thread), NULL, tshirt, (void *)cords[i]);
                if (k)
                {
                    perror("Failed to create thread");
                }
                //break;
                pthread_join(cords[i]->cord_thread, NULL);
                //performers[performer->id]->mute_id = i;
                //sem_post(&mutex);
                break;
            }
        }
        //break;
        //printf("stop!\n");
    }
}


void *thread(void *args)
{
    Performer *performer = (Performer *)args;
    int random;
    sem_wait(&mutex);
    if(t1!=t2)
        random = rand() % (t2 - t1) + t1 + 1;
    else
        random = t1;
    int stop = min_time;
    int flag = 0;
    sem_post(&mutex);

    if(performer->stage=='a')
        printf("%s performing %c at acoustic stage %d for %d sec.\n", performer->name, performer->ins, performer->stage_id, random);
    else if (performer->stage == 'e')
        printf("%s performing %c at electric stage %d for %d sec.\n", performer->name, performer->ins, performer->stage_id, random);

    while (min_time < stop + random)
    {
        //printf("stop+random: %d", stop + random);
        if (performer->stage == 'a' && astages[performer->stage_id]->sing == 1 && flag==0)
        {
            sem_wait(&mutex);
            flag=1;
            sem_post(&mutex);
        }
        else if (performer->stage == 'e' && estages[performer->stage_id]->sing == 1)
        {
            sem_wait(&mutex);
            flag = 1;
            sem_post(&mutex);
        }
    }
    if(flag==1)
    {
        while(min_time < stop+random+2)
            ;
    }

    if (performer->stage == 'a')
        printf("%s performance at acoustic stage ended.\n", performer->name);
    else
        printf("%s performance at electric stage ended.\n", performer->name);
    sem_wait(&mutex);
    
    //done_students++;
    
    if(performer->stage=='a')
    {
        astages[performer->stage_id]->music= 0;
        astages[performer->stage_id]->music_id = -1;
    }
    else
    {
        estages[performer->stage_id]->music = 0;
        estages[performer->stage_id]->music_id = -1;
    }
    sem_post(&mutex);
    //printf("mute_id %d\n", performer->mute_id);
    //sem_post(&mutex);
    pthread_create(&(cor_thread), NULL, coord, (void *)performers[performer->id]);
}

void *stage(void *args)
{
    //for (int i = 0; i < k; i++)
    //{
    int* hello = (int*)args;
    int i = *hello;
    //printf("%d %d\n", min_time, i);
    if (performers[i]->status == 1)
    {
        //printf("%s  status 1||\n", performers[i]->name);
        return NULL;
    }
    //printf("%d %d\n", performers[i]->active_time, timer);
    if (performers[i]->active_time >= timer && performers[i]->status == 0)
    {
        sem_wait(&muter);
        performers[i]->status = 1;
        done_students++;
        sem_post(&muter);
        printf("%s %c left because of impatience.\n", performers[i]->name, performers[i]->ins);
        //printf("%s  bye||\n", performers[i]->name);
        return NULL;
    }

    if (performers[i]->status == 0)
    {
        //printf("hi\n");
        sem_wait(&muter);
        performers[i]->active_time++;
        sem_post(&muter);
        //printf("hi\n");
    }
    if (performers[i]->time != min_time)
    {
        //printf("%s---Its not my time!!\n", performers[i]->name);
        return NULL;
    }

    if (performers[i]->arrival == 0)
    {
        sem_wait(&muter);
        performers[i]->arrival = 1;
        sem_post(&muter);
        printf("%s %c arrived.\n", performers[i]->name, performers[i]->ins);
    }
    //sem_wait(&muter);
    for (int j = 0; j < a; j++)
    {
        if (performers[i]->ins == 'b')
            break;
        if (performers[i]->ins != 's' && astages[j]->music == 0)
        {
            sem_wait(&muter);
            performers[i]->status = 1;
            astages[j]->music = 1;
            astages[j]->music_id = i;
            performers[i]->stage = 'a';
            performers[i]->stage_id = j;
            performers[i]->active_time--;
            sem_post(&muter);
            pthread_create(&(astages[j]->a_thread), NULL, thread, (void *)performers[i]);
            break;
        }
        else if (performers[i]->ins == 's' && astages[j]->sing == 0)
        {
            sem_wait(&muter);
            performers[i]->status = 1;
            astages[j]->sing = 1;
            //astages[j]->music_id = i;
            performers[i]->stage = 'a';
            performers[i]->stage_id = j;
            performers[i]->active_time--;
            sem_post(&muter);
            pthread_create(&(astages[j]->a_thread), NULL, thread_imp, (void *)performers[i]);
            break;
        }
    }
    if (performers[i]->status == 0)
    {
        for (int j = 0; j < e; j++)
        {
            if (performers[i]->ins == 'v')
                break;
            if (performers[i]->ins != 's' && estages[j]->music == 0)
            {
                sem_wait(&muter);
                performers[i]->status = 1;
                estages[j]->music = 1;
                estages[j]->music_id = i;
                performers[i]->stage = 'e';
                performers[i]->stage_id = j;
                performers[i]->active_time--;
                sem_post(&muter);
                pthread_create(&(estages[j]->e_thread), NULL, thread, (void *)performers[i]);
                break;
            }
            else if (performers[i]->ins == 's' && estages[j]->sing == 0)
            {
                sem_wait(&muter);
                performers[i]->status = 1;
                estages[j]->sing = 1;
                //estages[j]->music_id = i;
                performers[i]->stage = 'e';
                performers[i]->stage_id = j;
                performers[i]->active_time--;
                sem_post(&muter);
                pthread_create(&(estages[j]->e_thread), NULL, thread_imp, (void *)performers[i]);
                break;
            }
        }
    }
    //sem_post(&muter);
    //sleep(1);
    
}

int main()
{
    int ask = 0;
    scanf("%d %d %d %d %d %d %d", &k, &a, &e, &fly, &t1, &t2, &timer);
    if(a==0 && e==0)
    {
        printf("No stages to perform!!\n");
        printf("Finished.\n");
        return 0;
    }
   
    performers = (Performer **)malloc(k * sizeof(Performer *));
    astages = (a_stage **)malloc(a * sizeof(a_stage *));
    estages = (e_stage **)malloc(e * sizeof(e_stage *));
    cords = (Cord **)malloc(fly * sizeof(Cord *));
    char input[120], c;
    int x, tim, fact = 1, ans, max_time;
    
    sem_init(&mutex, 0, 1);
    sem_init(&muts, 0, 1);

    for (int i = 0; i < c; i++)
        sem_init(&mute[i], 0, 1);
    
    for (int i = 0; i < 100;i++)
        pthread_mutex_init(&tex[i], NULL);
    
    sem_init(&muter, 0, 1);

    for (int i = 0; i < k; i++)
    {
        x = 0, fact = 1, ans = 0;
        performers[i] = performer_initialize(i, performers[i]);

        scanf("%s", input);
        strcat(performers[i]->name, input);
        scanf(" %c", &c);
        performers[i]->ins = c;
        if(c!='s')
            ask++;
        scanf("%s", input);
        for (int j = 0;; j++)
        {
            if(input[j]=='\0')
                break;
            ans += (input[j] - '0') * fact;
            fact *= 10;
        }
        performers[i]->time = ans;
        if(ans>max_time)
            max_time = ans;
        if(ans<min_time)
            min_time = ans;
        //printf("%s %c %d\n", performers[i]->name, performers[i]->ins, performers[i]->time);
    }

    if(fly==0 && ask!=0)
    {
        printf("No co-ordinators!!\n");
        printf("Finished.\n");
        return 0;
    }

    for (int i = 0; i < a; i++)
    {
        astages[i] = astage_initialize(i, astages[i]);
    }

    for (int i = 0; i < e; i++)
    {
        estages[i] = estage_initialize(i, estages[i]);
    }

    for (int i = 0; i < fly; i++)
    {
        cords[i] = cord_initialize(i, cords[i]);
    }

    srand(time(0));
    min_time=0;
    int point[k];
    //printf("min %d max %d\n", min_time, max_time);
    //printf("%d\n", timer);
    while (done_students < k)
    {
        //sleep(1);
        for (int i = 0; i < k; i++)
        {
            point[i] = i;
            int *points = &point[i];
            int k = pthread_create(&(performers[i]->performer_thread), NULL, stage, (void *)points);
        }
        
        sleep(1);
        min_time++;
        
    }
    for (int i = 0; i < k; i++)
    {
        pthread_join(performers[i]->performer_thread, NULL);
    }
    for (int i = 0; i < e; i++)
        pthread_join(estages[i]->e_thread, NULL);
    for (int i = 0; i < a; i++)
        pthread_join(astages[i]->a_thread, NULL);
    //for (int i = 0; i < fly; i++)
      //pthread_join(cords[i]->cord_thread, NULL);
    sem_destroy(&mutex);
    for (int i = 0; i < c; i++)
        sem_destroy(&mute[i]);
    sem_destroy(&muts);
    sem_destroy(&muter);
    for (int i = 0; i < 100; i++)
        pthread_mutex_destroy(&tex[i]);
    printf("Finished.\n");
    return 0;
}