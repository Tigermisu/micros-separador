#ifndef SEPARADOR_HEADER
#define	SEPARADOR_HEADER

// Define enums to control state machine
typedef enum {WAITING, REDIRECT, CONTAINER_CONFIG} SeparationStates;
typedef enum {DEFAULT, SELECTING, SWEEPING, RESETTING} RedirectionStates;

// Define bit fields to store garbage container status
struct {
    unsigned containerGeneral: 1;
    unsigned containerAluminium: 1;
    unsigned containerPaper: 1;
    unsigned containerPlastic: 1;
} ContainerStatus;

void initialize();

void initSFRs();

void stateMachine();

void initContainers();

SeparationStates listenForInitialInputs();

char detectTargetContainer();

char positionHole(char tgtContainer);

char sweepItem();

char resetSeparator();

void showMessage(char message[]);

void enqueueMessage(char message[]);

void consumeMessageQueue(long nextMinTime);

void queryContainerCapacity(char containerNumber);

void displayADResults();

#endif

