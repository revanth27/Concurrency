#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <pthread.h>

int *ark;

pthread_mutex_t mutex;
typedef struct node
{
    int l, r;
}node;

void s_sort(int *arr, int l, int r)
{
    int min_idx, temp;
    //printf("h\n");
    for (int i = l; i < r; i++)
    {
        min_idx = i;
        for (int j = i + 1; j <= r; j++)
            if (arr[j] < arr[min_idx])
                min_idx = j;

        temp = arr[min_idx];
        arr[min_idx] = arr[i];
        arr[i] = temp;
    }
}

void merge(int *arr, int l, int mid, int r)
{
    int len1 = mid - l + 1;
    int len2 = r - mid;
    int arr1[len1], arr2[len2];

    for (int i = 0; i < len1; i++)
        arr1[i] = arr[l + i];
    for (int i = 0; i < len2; i++)
        arr2[i] = arr[mid + 1 + i];

    int i = 0, j = 0, k = l;

    while(i < len1 && j < len2)
    {
        if(arr1[i] < arr2[j])
            arr[k++] = arr1[i++];
        else
            arr[k++] = arr2[j++];
    }
    while(i < len1)
        arr[k++] = arr1[i++];
    while(j<len2)
        arr[k++] = arr2[j++];
}

void mergesort(int *arr, int l, int r)
{
    //printf("h\n");
    if(l >= r)
        return;
    
    if((r - l + 1) < 5)
    {
        s_sort(arr, l, r);
        return;
    }

    int mid = l + (r - l) / 2;

    pid_t pid1, pid2;
    pid1 = fork();

    if(pid1 < 0)
    {
        perror("fork");
        _exit(1);
    }
    else if(!pid1)
    {
        mergesort(arr, l, mid);
        _exit(0);
    }
    else
    {
        pid2 = fork();
        if(pid2 < 0)
        {
            perror("fork");
            _exit(1);
        }
        else if(!pid2)
        {
            mergesort(arr, mid+1, r);
            _exit(0);
        }
        else
        {
            int status;
            waitpid(pid1, &status, 0);
            waitpid(pid2, &status, 0);
            merge(arr, l, mid, r);
        }
    }
}


void mergesort_normal(int *ar, int l, int r)
{
    if(l>=r)
        return;

    if ((r - l + 1) < 5)
    {
        s_sort(ar, l, r);
        return;
    }

    int mid = l + (r - l) / 2;
    mergesort_normal(ar, l, mid);
    mergesort_normal(ar, mid + 1, r);
    merge(ar, l, mid, r);
}

void *mergesort_thread(void *args)
{
    node *elem = (node *)args;
    int l = elem->l;
    int r = elem->r;
    if (l >= r)
    {
        pthread_exit(0);
    }
    if ((r - l + 1) < 5)
    {
        pthread_mutex_lock(&mutex);
        s_sort(ark, l, r);
        pthread_mutex_unlock(&mutex);
        pthread_exit(0);
    }
    node left, right;
    int m = (l + r) / 2;
    pthread_t leftId, rightId;
    left.l = l;
    left.r = m;
    right.l = m + 1;
    right.r = r;
    pthread_create(&leftId, NULL, mergesort_thread, &left);
    pthread_create(&rightId, NULL, mergesort_thread, &right);
    pthread_join(leftId, NULL);
    pthread_join(rightId, NULL);
    pthread_mutex_lock(&mutex);
    merge(ark, l, m, r);
    pthread_mutex_unlock(&mutex);
    pthread_exit(0);
}

int main()
{
    int n, id;
    scanf("%d", &n);
    int *arr, ar[n];
    ark = (int *)malloc(n * sizeof(int));
    //time_t begin, end;

    if((id = shmget(IPC_PRIVATE, sizeof(int)*(n), IPC_CREAT | 0666))<0)
    {
        perror("shmget");
        _exit(1);
    }

    if(!(arr = shmat(id, NULL, 0)))
    {
        perror("shmat");
        _exit(1);
    }

    for (int i = 0; i < n; i++)
    {
        scanf("%d", &ar[i]);
        arr[i] = ar[i];
        ark[i] = ar[i];
    }

    printf("\nConcurrent merge sort\n");
    clock_t begin = clock();

    mergesort(arr, 0, n - 1);

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);
    printf("\nTime taken: %lf", time_spent);

    //printf("h\n");
    if (shmdt(arr) == -1)
    {
        perror("shmdt");
        _exit(1);
    }

    if(shmctl(id, IPC_RMID, NULL)==-1)
    {
        perror("shmctl");
        _exit(1);
    }

    printf("\n\nNormal merge sort\n");
    begin = clock();

    mergesort_normal(ar, 0, n-1);

    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    
    for (int i = 0; i < n; i++)
        printf("%d ", ar[i]);

    printf("\nTime taken: %lf\n", time_spent);

    printf("\nConcurrent merge sort using threads\n");
    node temp;
    temp.l = 0;
    temp.r = n - 1;
    pthread_t root;
    begin = clock();
    pthread_create(&root, NULL, mergesort_thread, &temp);
    pthread_join(root, NULL);
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    for (int i = 0; i < n; i++)
    {
        printf("%d ", ark[i]);
    }
    printf("\nTime taken: %lf\n", time_spent);
    return 0;
}