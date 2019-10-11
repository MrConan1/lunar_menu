#ifndef NODE_TYPES
#define NODE_TYPES


/* Struct Forward Declarations */
typedef struct scriptNode scriptNode;

/* Script Node Datatype */
struct scriptNode
{
    unsigned int id;
    int type;
    int numWdArgs;

    char* data;
    char* data2;
    int dataSizeBytes;
    int dataSizeBytes2;
    short offsetArray[10];
    int offsetElements;

    /* Original Location Data */
    int originalStartOffset;
    int childAStartOffset;
    int childBStartOffset;

    /* Updated Location Data */
    int modifiedStartOffset;
    int modifiedChildAStartOffset;
    int modifiedChildBStartOffset;

    scriptNode* pNextA;
    scriptNode* pNextB;
    scriptNode* pPrev;
};


/* Identifiers for Nodes not associated with a Script Command */
#define NODE_UNKNOWN    0xFFFF
#define NODE_INDEX      0x0000

/* Actual Script Commands */
#define NODE_TEXT       0x0002  /* Text */
#define NODE_U_JMP      0x0003  /* Unconditional Jump */
#define NODE_EOS        0x0005  /* End of Current Script Sequence */
#define NODE_SEL_OPT    0x0007  /* Select Between 1 of 2 Options  */
#define NODE_SET_B      0x0009  /* Set Bit */
#define NODE_CLR_B      0x000A  /* Clear Bit */
#define NODE_CT_JMP     0x000B  /* Jump Conditional True  */
#define NODE_CF_JMP     0x000C  /* Jump Conditional False */
#define NODE_DYN1       0x0026
#define NODE_COWSHED1   0x0027
#define NODE_DYN2       0x002A
#define NODE_STATUE     0x002F
#define NODE_COWSHED2   0x0031
#define NODE_COWSHED3   0x0032
#define NODE_COWSHED4   0x0038
#define NODE_DYN3       0x003E
#define NODE_COWSHED5   0x0045
#define NODE_COWSHED6   0x0047
#define NODE_B4_AUDIO0  0x0052
#define NODE_B4_AUDIO1  0x0053
#define NODE_AUDIO      0x0054
#define NODE_B4_AUDIO2  0x005A


#endif
