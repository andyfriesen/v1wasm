// Single-byte opcode descriptors

#define EXEC 1
#define VAR0_ASSIGN 2
#define VAR1_ASSIGN 3
#define VAR2_ASSIGN 4
#define GENERAL_IF 5
#define ELSE 6
#define GOTO 7
#define FOR_LOOP0 8
#define FOR_LOOP1 9
#define SWITCH 10
#define CASE 11
#define ENDSCRIPT 255

// Single-byte operand descriptors

#define OP_IMMEDIATE 1
#define OP_VAR0 2
#define OP_VAR1 3
#define OP_VAR2 4
#define OP_GROUP 5

// Single-byte IF handler parameters

#define ZERO 0
#define NONZERO 1
#define EQUALTO 2
#define NOTEQUAL 3
#define GREATERTHAN 4
#define GREATERTHANOREQUAL 5
#define LESSTHAN 6
#define LESSTHANOREQUAL 7

// Single byte assignment descriptors

#define SET 1
#define INCREMENT 2
#define DECREMENT 3
#define INCSET 4
#define DECSET 5

// Operand combination descriptors
#define ADD 1
#define SUB 2
#define MULT 3
#define DIV 4
#define MOD 5
#define OP_END 255
