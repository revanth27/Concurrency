#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int counter = 0, done_students = 0;
pthread_mutex_t lock;
int n, o, m, error, prev = -1, lar, hel = 0, mins;
int ini[1000] = {0};
int final[1000] = {-1}, top = 0;
pthread_mutex_t mutex;


typedef struct Vzone
{
    int id;
    int slots;
    //int status;
    int actual;
    int capacity;
    int mins;
    int lar;
    int hel;
    int iter;
    pthread_t vzone_thread;
    pthread_mutex_t mutex_vzone;
} Vzone;

typedef struct Company
{
    int id;
    pthread_t company_thread;
    float x;
    int total;
    int success;
    int capacity;
    int batches;
    Vzone *vzone_related;
} Company;

typedef struct Student
{
    int id;
    int round;
    int status;
    pthread_t student_thread;
    Vzone *vzone_related;
    Company *company_related;
}Student;


Vzone *vzone_initialize(int id, Vzone* vzone)
{
    vzone = (Vzone *)malloc(sizeof(Vzone));
    vzone->id = id;
    //vzone->status = 0;
    vzone->hel = 0;
    vzone->iter = 0;
    vzone->actual = 0;
    vzone->capacity = 0;
    pthread_mutex_init(&(vzone->mutex_vzone), NULL);
    return vzone;
}

Student* student_initialize(int id, Student* student)
{
    student = (Student *)malloc(sizeof(Student));
    student->id= id;
    student->round = 1;
    student->status = 0;

    //student->vzone_related = vzone;
    return student;
}

Company* company_intialize(int id, Company* company)
{
    company = (Company *)malloc(sizeof(Company));
    company->id=id;
    company->success = 0;
    company->total = 1;
    company->batches = 0;
    return company;
}

Vzone **zones;
Student **students;
Company **companys;

void *student_thread(void *args)
{
    //pthread_mutex_lock(&mutex);
    Student *student = (Student *)args;
    int stuid = student->id;
    Company *company = (Company *)student->company_related;
    Vzone *vzone = (Vzone *)company->vzone_related;
    int h = company->id;
    float y = company->x;
    //printf("total: %d\n", companys[h]->total);
    int prob = (int)(y * companys[h]->total);
    //pthread_mutex_lock(&mutex);
    if(prob > companys[h]->success)
    {
        //printf("h\n");
        companys[h]->success++;
        if (students[stuid]->status == 0)
            done_students++;
        students[stuid]->status = 1;
        printf("Student %d has tested positive for antibodies.\n", stuid);
    }
    else if(students[stuid]->round==3)
    {
        if (students[stuid]->status == 0)
            done_students++;
        students[stuid]->status = 1;
        printf("Student %d has tested negative for antibodies.\n", stuid);
    }
    else
    {
        ini[stuid] = 0;
        counter--;
        students[stuid]->round++;
        printf("Student %d has tested negative for antibodies.\n", stuid);
    }
    //pthread_mutex_unlock(&mutex);
    return NULL;
}

void* company_thread(void *args)
{
    Company *company = (Company *)args;
    int h = company->id;
    Vzone *vzone = (Vzone *)companys[h]->vzone_related;
    int ri = 1 + rand() % 5;
    companys[h]->batches = ri;
    printf("Pharmaceutical company %d is preparing %d batches of vaccines which have success probability %f.\n", h, ri, companys[h]->x);
    int q = vzone->id;
    int w = 2 + rand() % 4;
    sleep(w);
    int p = 1 + rand() % 21;
    companys[h]->capacity = p;
    zones[q]->capacity = p;
    printf("Pharmaceutical company %d prepared %d batches of vaccines which have success probability %f.\n", h, companys[h]->batches, companys[h]->x);
    printf("Pharmaceutical Company %d is delivering a vaccine batch to Vaccination Zone %d which has success probability %f.\n", h, q, companys[h]->x);
    return NULL;
}
int r, store;
void* vzone_thread(void* args)
{
    Vzone *vzone = (Vzone *)args;
    while(1)
    {
        
        sleep(1);
        
        pthread_mutex_lock(&mutex);
        printf("done_students: %d\n", done_students);
        if (done_students == o)
        {
            pthread_mutex_unlock(&mutex);
            break;
        }
        //pthread_mutex_unlock(&mutex);
        //pthread_mutex_lock(&mutex);
        if (vzone->capacity == 0)
        {
            //pthread_mutex_lock(&mutex);
            printf("Vaccination zone %d has run out of vaccines.\n", vzone->id);
            r = rand() % n;
            companys[r]->vzone_related = vzone;
            if (companys[r]->batches == 0 && vzone->actual == 0)
            {
                printf("All the vaccines prepared by Pharmaceutical Company %d are emptied. Resuming production now.\n", r);
                sleep(1);
                int k = pthread_create(&(companys[r]->company_thread), NULL, company_thread, (void *)companys[r]);
                if (k)
                {
                    perror("Failed to create thread");
                }
                pthread_join(companys[r]->company_thread, NULL);
                companys[r]->batches--;
                //companys[r]->total = 1;
                //companys[r]->success = 0;
                printf("Pharmaceutical company %d delivered a vaccine batch to Vaccination Zone %d which have success probability %f.\n", r, vzone->id, companys[r]->x);
                vzone->actual = companys[r]->capacity;
                vzone->capacity = companys[r]->capacity;
                //vzone->capacity = rand() % (companys[r]->capacity) + 1;
                if (vzone->capacity > 8)
                    vzone->capacity = 8;
                vzone->actual -= vzone->capacity;
            }
            else if(vzone->actual!=0)
            {
                vzone->actual = vzone->capacity;
                if (vzone->capacity > 8)
                    vzone->capacity = 8;
                vzone->actual -= vzone->capacity;
            }
            else
            {
                companys[r]->batches--;
                vzone->capacity = companys[r]->capacity;
                //vzone->capacity = rand() % (companys[r]->capacity) + 1;
                if (vzone->capacity > 8)
                    vzone->capacity = 8;
                vzone->actual -= vzone->capacity;
            }
            //pthread_mutex_unlock(&mutex);
            printf("Vaccination zone %d is ready to vaccinate with %d slots.\n", vzone->id, vzone->capacity);
        }



        //pthread_mutex_unlock(&mutex);
        //int mins;

        if(vzone->iter==0)
        {
            //pthread_mutex_lock(&mutex);
            
            prev = hel;
            mins = top - prev;
            if (vzone->capacity < mins)
                mins = vzone->capacity;
            //printf("I am here %d\n", mins);
            lar = prev + mins;
            hel = prev + mins;
            zones[vzone->id]->hel = hel;
            zones[vzone->id]->lar = prev;
            zones[vzone->id]->mins = mins;
            if (prev == -1)
                prev = 0;
            //pthread_mutex_unlock(&mutex);
            for (int i = prev; i < prev + mins; i++)
                printf("Student %d assigned a slot on the vaccination zone %d and waiting to be vaccinated.\n", final[i], vzone->id);
            if (zones[vzone->id]->mins != 0)
                printf("Vaccination zone %d entering Vaccination phase.\n", vzone->id);
        }
        

        //int lar = top;
        //if(mins!=0)top -= 1;
        printf("mins: %d, zone id: %d, top: %d\n", zones[vzone->id]->mins, vzone->id, top);
        vzone->iter++;
        if (zones[vzone->id]->mins > 0)
        {
            students[final[zones[vzone->id]->lar]]->company_related = companys[r];
            students[final[zones[vzone->id]->lar]]->vzone_related = vzone;

            //sleep(1);
            printf("Student %d on Vaccination Zone %d has been vaccinated which has success probability %f.\n", final[zones[vzone->id]->lar], vzone->id, companys[r]->x);
            
            int k = pthread_create(&(students[final[zones[vzone->id]->lar]]->student_thread), NULL, student_thread, (void *)students[final[zones[vzone->id]->lar]]);
            if (k)
            {
                perror("Failed to create thread");
            }
            sleep(2);
            pthread_join(students[final[zones[vzone->id]->lar]]->student_thread, NULL);
            //sleep(2);

            zones[vzone->id]->lar++;
            //printf("lar %d\n", zones[vzone->id]->lar);
            vzone->capacity--;
            companys[r]->total++;
            zones[vzone->id]->mins--;
        }
        else
        {
            //vzone->capacity=0;
            vzone->iter = 0;
            //printf("ho\n");
        }
        
        //top = lar;
        pthread_mutex_unlock(&mutex);
    }
    //pthread_mutex_unlock(&mutex);
    return NULL;
}

int al = 0, star = 0, stop = 0, hi = 0, out = 0;
int main()
{
    scanf("%d %d %d", &n, &m, &o);
    if(n<0 || m<0 || o<0)
    {
        printf("Invalid arguments!!.\n");
        return 0;
    }
    if(n==0)
    {
        printf("No pharmaceutical companies!!\n");
        out = 1;
    }
    if(m==0)
    {
        printf("No vaccination zones.\n");
        out = 1;
    }
    if(o==0)
    {
        printf("No students.\n");
        out = 1;
    }
    if (out == 1)
    {
        printf("Simulation over.\n");
        return 0;
    }
    counter = 0, done_students=0;
    pthread_mutex_init(&mutex, NULL);
    companys = (Company **)malloc(sizeof(Company *) * n);
    zones = (Vzone **)malloc(sizeof(Vzone *) * m);
    students = (Student **)malloc(sizeof(Student *) * o);
    printf("Simulation started.\n");

    for (int i = 0; i < m; i++)
        zones[i] = vzone_initialize(i, zones[i]);

    for (int i = 0; i < o; i++)
        students[i] = student_initialize(i, students[i]);

    for (int i = 0; i < n; i++)
    {
        companys[i] = company_intialize(i, companys[i]);
        float p;
        scanf("%f", &p);
        companys[i]->x = p;
    }
    srand(time(0));

    for (int i = 0; i < m; i++)
    {
        int k = pthread_create(&(zones[i]->vzone_thread), NULL, vzone_thread, (void *)zones[i]);
        if (k)
        {
            perror("Failed to create thread");
        }
    }
    
    while(done_students != o)
    {
        int na;
        int time = rand() % 10 + 1;
        if ((al + time) % o < al)
            na = o - 1;
        else
            na = (al + time) % o;
        pthread_mutex_lock(&mutex);
        if(star==0)
        {
            stop = time;
            while(stop--)
            {
                if(hi==o){
                    star = 1;
                    break;
                }
                ini[hi] = 1;
                final[top++] = hi;
                counter++;
                hi++;
            }
            //printf("lllllllllllllllllllllllllllllllll\n");
            //sleep(2);
            //continue;
        }
        else{
        for (int j = 0; j < o; j++)
        {
            if (ini[j] == 0)
            {
                ini[j] = 1;
                counter++;
                final[top++] = j;
                printf("Student %d has arrived for his %d round of vaccination.\n", j, students[j]->round);
                printf("Student %d is waiting to be allocated a slot in vaccination zone.\n", j);
            }
        }}
        pthread_mutex_unlock(&mutex);
        al += time;
        al %= o;
        //printf("time:%d\n", na);
        time = rand() % 2 + 4;
        sleep(2);
    }

    for (int i = 0; i < m; i++){
        pthread_join(zones[i]->vzone_thread, NULL);
        //printf("h\n");
    }

    /*for (int i = 0; i < o; i++)
        pthread_join(students[i]->student_thread, NULL);*/
    
    /*for (int i = 0; i < n; i++){
        pthread_join(companys[i]->company_thread, NULL);
        printf("s\n");
    }
    */
    
    //printf("%d\n", top);
    pthread_mutex_destroy(&mutex);
    for (int i = 0; i < m; i++)
        pthread_mutex_destroy(&(zones[i]->mutex_vzone));
    printf("Simulation over.\n");

    return 0;
}