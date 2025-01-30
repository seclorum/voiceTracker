/*
 *
 * A simple synthesizer voice-assignment implementation.
 * (c) seclorun 2025
 *
 * MIT Licensed - see LICENSE
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define MAX_VOICES 8
#define NOTE_ON  1
#define NOTE_OFF 0

// Timestamp in nanoseconds
long long current_time_ns() {
#ifdef _WIN32
    static LARGE_INTEGER frequency;
    static int initialized = 0;
    if (!initialized) {
        QueryPerformanceFrequency(&frequency);
        initialized = 1;
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (counter.QuadPart * 1000000000LL) / frequency.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (ts.tv_sec * 1000000000LL) + ts.tv_nsec;
#endif
}

// Notes being tracked are stored in a Deque
typedef struct Node {
    int note;
    struct Node* next;
} Node;

typedef struct {
    Node* front;
    Node* rear;
    int size;
} Deque;

Deque* createDeque() {
    Deque* dq = (Deque*)malloc(sizeof(Deque));
    dq->front = dq->rear = NULL;
    dq->size = 0;
    return dq;
}

bool isEmpty(Deque* dq) {
    return dq->size == 0;
}

// Check if a note already exists in the deque
bool noteExists(Deque* dq, int note) {
    Node* temp = dq->front;
    while (temp) {
        if (temp->note == note) return true;
        temp = temp->next;
    }
    return false;
}

void pushFront(Deque* dq, int note) {
    if (noteExists(dq, note)) return; // Prevent duplicate notes
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->note = note;
    newNode->next = dq->front;
    dq->front = newNode;
    if (dq->rear == NULL) dq->rear = newNode;
    dq->size++;
}

void pushBack(Deque* dq, int note) {
    if (noteExists(dq, note)) return; // Prevent duplicate notes
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->note = note;
    newNode->next = NULL;
    if (dq->rear != NULL) dq->rear->next = newNode;
    dq->rear = newNode;
    if (dq->front == NULL) dq->front = newNode;
    dq->size++;
}

int popBack(Deque* dq) {
    if (isEmpty(dq)) return -1;
    Node* temp = dq->front;
    Node* prev = NULL;
    while (temp->next) {
        prev = temp;
        temp = temp->next;
    }
    int note = temp->note;
    free(temp);
    if (prev) prev->next = NULL;
    else dq->front = NULL;
    dq->rear = prev;
    dq->size--;
    return note;
}

void print_deque_contents(Deque* dq) {
    Node* temp = dq->front;
    printf("Note state as of [%lld]: ", current_time_ns());
    if (!temp) {
        printf("None");
    } else {
        while (temp) {
            printf("%d", temp->note);
            if (temp->next) printf(", ");
            temp = temp->next;
        }
    }
    printf("\n");
}

// Set the state for a voice
void synth_voice_ts(int voicenum, int state, Deque* dq) {
    printf("Voice %d: [%lld ns]  %s \n", voicenum, current_time_ns(), state == 1 ? "NOTE ON" : "NOTE OFF" );
    print_deque_contents(dq);
}

void noteOn(Deque* dq, int note) {
    if (dq->size >= MAX_VOICES) {
        int stolenNote = popBack(dq);
        if (stolenNote != -1) {
            synth_voice_ts(stolenNote, NOTE_OFF, dq);
        }
    }
    pushFront(dq, note);
    synth_voice_ts(note, NOTE_ON, dq);
}

void removeNote(Deque* dq, int note) {
    Node* temp = dq->front;
    Node* prev = NULL;
    while (temp) {
        if (temp->note == note) {
            if (prev) prev->next = temp->next;
            else dq->front = temp->next;

            if (temp == dq->rear) {
                dq->rear = prev;
            }

            free(temp);
            dq->size--;
            return;
        }
        prev = temp;
        temp = temp->next;
    }
}

void noteOff(Deque* dq, int note) {
    removeNote(dq, note);
    synth_voice_ts(note, NOTE_OFF, dq);
}

void freeDeque(Deque* dq) {
    while (!isEmpty(dq)) popBack(dq);
    free(dq);
}

void dumpDeque(Deque* dq) {
    while (!isEmpty(dq)) {
        printf("note: %d\n", popBack(dq));
    }
}

void random_note_event(Deque* dq) {
    int note = (rand() % 13) + 60;
    int state = rand() % 2;

    if (state == NOTE_ON) {
        noteOn(dq, note);
    } else {
        noteOff(dq, note);
    }
}

int main() {
    Deque* dq = createDeque();
    int cnt = 0;

    // Test 1 - known values
    noteOn(dq, 60);
    noteOn(dq, 62);
    noteOn(dq, 64);
    noteOn(dq, 65);
    noteOn(dq, 67);
    noteOn(dq, 69);
    noteOn(dq, 71);
    noteOn(dq, 72);
    noteOn(dq, 74);
    noteOff(dq, 69);
    noteOff(dq, 64);
    noteOff(dq, 65);
    noteOn(dq, 69);
    noteOn(dq, 64);
    noteOn(dq, 39);
    noteOn(dq, 49);
    noteOn(dq, 89);
    noteOff(dq, 69);
    noteOn(dq, 99);
    noteOff(dq, 49);

    // Test 2 - 20 random values
    while (cnt++ < 20) {
        random_note_event(dq);
    }

    dumpDeque(dq);
    return 0;
}

