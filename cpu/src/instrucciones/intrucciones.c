#include "intrucciones.h"
void NOOP()
{
    contexto_ejecucion.registros.PC += 1;
}

void SET(reg_id reg, uint32_t valor)
{
    switch (reg)
    {
    case AX:
        contexto_ejecucion.registros.AX = (uint8_t)valor;
        break;
    case BX:
        contexto_ejecucion.registros.BX = (uint8_t)valor;
        break;
    case CX:
        contexto_ejecucion.registros.CX = (uint8_t)valor;
        break;
    case DX:
        contexto_ejecucion.registros.DX = (uint8_t)valor;
        break;

    case PC:
        contexto_ejecucion.registros.PC = valor;
        break;
    case EAX:
        contexto_ejecucion.registros.EAX = valor;
        break;
    case EBX:
        contexto_ejecucion.registros.EBX = valor;
        break;
    case ECX:
        contexto_ejecucion.registros.ECX = valor;
        break;
    case EDX:
        contexto_ejecucion.registros.EDX = valor;
        break;
    case SI:
        contexto_ejecucion.registros.SI = valor;
        break;
    case DI:
        contexto_ejecucion.registros.DI = valor;
        break;
    }
}

// TODO mov_in
// void MOV_IN(reg_id reg)
// {
//     uint32_t dir = contexto_ejecucion.registros.SI;

//     switch (reg)
//     {
//     // Registros de 8 bits
//     case AX:
//         contexto_ejecucion.registros.AX = *((uint8_t *)memoria + dir);
//         break;

//     case BX:
//         contexto_ejecucion.registros.BX = *((uint8_t *)memoria + dir);
//         break;

//     case CX:
//         contexto_ejecucion.registros.CX = *((uint8_t *)memoria + dir);
//         break;

//     case DX:
//         contexto_ejecucion.registros.DX = *((uint8_t *)memoria + dir);
//         break;

//     // Registros de 32 bits (forma segura)
//     case EAX:
//         memcpy(contexto_ejecucion.registros.EAX, (uint8_t *)memoria + dir, sizeof(uint32_t));
//         break;

//     case EBX:
//         memcpy(contexto_ejecucion.registros.EBX, (uint8_t *)memoria + dir, sizeof(uint32_t));
//         break;

//     case ECX:
//         memcpy(contexto_ejecucion.registros.ECX, (uint8_t *)memoria + dir, sizeof(uint32_t));
//         break;

//     case EDX:
//         memcpy(contexto_ejecucion.registros.EDX, (uint8_t *)memoria + dir, sizeof(uint32_t));
//         break;
//     case SI:
//         memcpy(contexto_ejecucion.registros.SI, (uint8_t *)memoria + dir, sizeof(uint32_t));
//         break;

//     case DI:
//         memcpy(contexto_ejecucion.registros.DI, (uint8_t *)memoria + dir, sizeof(uint32_t));
//         break;

//     case PC:
//         memcpy(contexto_ejecucion.registros.PC, (uint8_t *)memoria + dir, sizeof(uint32_t));
//         break;
//     }
// }
// TODO mov_in

// void MOV_OUT()
// {
//     uint32_t dir = contexto_ejecucion.registros.DI;

//     switch (reg)
//     {
//     // Registros de 8 bits
//     case AX:
//         contexto_ejecucion.registros.AX = *((uint8_t *)memoria + dir);
//         break;

//     case BX:
//         contexto_ejecucion.registros.BX = *((uint8_t *)memoria + dir);
//         break;

//     case CX:
//         contexto_ejecucion.registros.CX = *((uint8_t *)memoria + dir);
//         break;

//     case DX:
//         contexto_ejecucion.registros.DX = *((uint8_t *)memoria + dir);
//         break;

//     // Registros de 32 bits (forma segura)
//     case EAX:
//         memcpy(contexto_ejecucion.registros.EAX, (uint8_t *)memoria + dir, sizeof(uint32_t));
//         break;

//     case EBX:
//         memcpy(contexto_ejecucion.registros.EBX, (uint8_t *)memoria + dir, sizeof(uint32_t));
//         break;

//     case ECX:
//         memcpy(contexto_ejecucion.registros.ECX, (uint8_t *)memoria + dir, sizeof(uint32_t));
//         break;

//     case EDX:
//         memcpy(contexto_ejecucion.registros.EDX, (uint8_t *)memoria + dir, sizeof(uint32_t));
//         break;
//     case SI:
//         memcpy(contexto_ejecucion.registros.SI, (uint8_t *)memoria + dir, sizeof(uint32_t));
//         break;

//     case DI:
//         memcpy(contexto_ejecucion.registros.DI, (uint8_t *)memoria + dir, sizeof(uint32_t));
//         break;

//     case PC:
//         memcpy(contexto_ejecucion.registros.PC, (uint8_t *)memoria + dir, sizeof(uint32_t));
//         break;
//     }
// }

void SUM(reg_id dest, reg_id orig)
{
    // DESTINO 8 bits

    switch (dest)
    {
    case AX:
        switch (orig)
        {
        case AX:
            contexto_ejecucion.registros.AX += contexto_ejecucion.registros.AX;
            break;
        case BX:
            contexto_ejecucion.registros.AX += contexto_ejecucion.registros.BX;
            break;
        case CX:
            contexto_ejecucion.registros.AX += contexto_ejecucion.registros.CX;
            break;
        case DX:
            contexto_ejecucion.registros.AX += contexto_ejecucion.registros.DX;
            break;
        default:
            break;
        }
        break;

    case BX:
        switch (orig)
        {
        case AX:
            contexto_ejecucion.registros.BX += contexto_ejecucion.registros.AX;
            break;
        case BX:
            contexto_ejecucion.registros.BX += contexto_ejecucion.registros.BX;
            break;
        case CX:
            contexto_ejecucion.registros.BX += contexto_ejecucion.registros.CX;
            break;
        case DX:
            contexto_ejecucion.registros.BX += contexto_ejecucion.registros.DX;
            break;
        default:
            break;
        }
        break;

    // DESTINO 32 bits
    case EAX:
        switch (orig)
        {
        case EAX:
            contexto_ejecucion.registros.EAX += contexto_ejecucion.registros.EAX;
            break;
        case EBX:
            contexto_ejecucion.registros.EAX += contexto_ejecucion.registros.EBX;
            break;
        case ECX:
            contexto_ejecucion.registros.EAX += contexto_ejecucion.registros.ECX;
            break;
        case EDX:
            contexto_ejecucion.registros.EAX += contexto_ejecucion.registros.EDX;
            break;
        case SI:
            contexto_ejecucion.registros.EAX += contexto_ejecucion.registros.SI;
            break;
        case DI:
            contexto_ejecucion.registros.EAX += contexto_ejecucion.registros.DI;
            break;
        case PC:
            contexto_ejecucion.registros.EAX += contexto_ejecucion.registros.PC;
            break;
        default:
            break;
        }
        break;

    case EBX:
        switch (orig)
        {
        case EAX:
            contexto_ejecucion.registros.EBX += contexto_ejecucion.registros.EAX;
            break;
        case EBX:
            contexto_ejecucion.registros.EBX += contexto_ejecucion.registros.EBX;
            break;
        case ECX:
            contexto_ejecucion.registros.EBX += contexto_ejecucion.registros.ECX;
            break;
        case EDX:
            contexto_ejecucion.registros.EBX += contexto_ejecucion.registros.EDX;
            break;
        case SI:
            contexto_ejecucion.registros.EBX += contexto_ejecucion.registros.SI;
            break;
        case DI:
            contexto_ejecucion.registros.EBX += contexto_ejecucion.registros.DI;
            break;
        case PC:
            contexto_ejecucion.registros.EBX += contexto_ejecucion.registros.PC;
            break;
        default:
            break;
        }
        break;
    }
}

void SUB(reg_id orig, reg_id dest)
{
    switch (dest)
    {
    // DESTINO 8 bits
    case AX:
        switch (orig)
        {
        case AX:
            contexto_ejecucion.registros.AX -= contexto_ejecucion.registros.AX;
            break;
        case BX:
            contexto_ejecucion.registros.AX -= contexto_ejecucion.registros.BX;
            break;
        case CX:
            contexto_ejecucion.registros.AX -= contexto_ejecucion.registros.CX;
            break;
        case DX:
            contexto_ejecucion.registros.AX -= contexto_ejecucion.registros.DX;
            break;
        default:
            break;
        }
        break;

    case BX:
        switch (orig)
        {

        case AX:
            contexto_ejecucion.registros.BX -= contexto_ejecucion.registros.AX;
            break;
        case BX:
            contexto_ejecucion.registros.BX -= contexto_ejecucion.registros.BX;
            break;
        case CX:
            contexto_ejecucion.registros.BX -= contexto_ejecucion.registros.CX;
            break;
        case DX:
            contexto_ejecucion.registros.BX -= contexto_ejecucion.registros.DX;
            break;
        default:
            break;
        }
        break;

    // DESTINO 32 bits
    case EAX:
        switch (orig)
        {
        case EAX:
            contexto_ejecucion.registros.EAX -= contexto_ejecucion.registros.EAX;
            break;
        case EBX:
            contexto_ejecucion.registros.EAX -= contexto_ejecucion.registros.EBX;
            break;
        case ECX:
            contexto_ejecucion.registros.EAX -= contexto_ejecucion.registros.ECX;
            break;
        case EDX:
            contexto_ejecucion.registros.EAX -= contexto_ejecucion.registros.EDX;
            break;
        case SI:
            contexto_ejecucion.registros.EAX -= contexto_ejecucion.registros.SI;
            break;
        case DI:
            contexto_ejecucion.registros.EAX -= contexto_ejecucion.registros.DI;
            break;
        case PC:
            contexto_ejecucion.registros.EAX -= contexto_ejecucion.registros.PC;
            break;
        default:
            break;
        }
        break;

    case EBX:
        switch (orig)
        {
        case EAX:
            contexto_ejecucion.registros.EBX -= contexto_ejecucion.registros.EAX;
            break;
        case EBX:
            contexto_ejecucion.registros.EBX -= contexto_ejecucion.registros.EBX;
            break;
        case ECX:
            contexto_ejecucion.registros.EBX -= contexto_ejecucion.registros.ECX;
            break;
        case EDX:
            contexto_ejecucion.registros.EBX -= contexto_ejecucion.registros.EDX;
            break;
        case SI:
            contexto_ejecucion.registros.EBX -= contexto_ejecucion.registros.SI;
            break;
        case DI:
            contexto_ejecucion.registros.EBX -= contexto_ejecucion.registros.DI;
            break;
        case PC:
            contexto_ejecucion.registros.EBX -= contexto_ejecucion.registros.PC;
            break;
        default:
            break;
        }
        break;
    }
}

void JNZ(reg_id registro, uint32_t instruccion)
{
    uint32_t valor_registro = 0;

    switch (registro)
    {
    case AX:
        valor_registro = contexto_ejecucion.registros.AX;
        break;
    case BX:
        valor_registro = contexto_ejecucion.registros.BX;
        break;
    case CX:
        valor_registro = contexto_ejecucion.registros.CX;
        break;
    case DX:
        valor_registro = contexto_ejecucion.registros.DX;
        break;
    case EAX:
        valor_registro = contexto_ejecucion.registros.EAX;
        break;
    case EBX:
        valor_registro = contexto_ejecucion.registros.EBX;
        break;
    case ECX:
        valor_registro = contexto_ejecucion.registros.ECX;
        break;
    case EDX:
        valor_registro = contexto_ejecucion.registros.EDX;
        break;
    case SI:
        valor_registro = contexto_ejecucion.registros.SI;
        break;
    case DI:
        valor_registro = contexto_ejecucion.registros.DI;
        break;
    default:
        log_debug(cpu_logger, "instruccion incorrecta en jnz");
        return;
    }

    /* info Si el valor del registro es distinto de cero, actualizamos el PC. (119) */
    if (valor_registro != 0)
    {
        contexto_ejecucion.registros.PC = instruccion;
    }
}
// TODO COPY_MEM
// void COPY_MEM(reg_id reg)
// {
//     uint32_t dir = contexto_ejecucion.registros.SI;

//     COPY_MEM(Registro Tamaño) : Toma la dirección apuntada por el registro SI y copia la cantidad de bytes
//                                     indicadas en el registro tamaño a la posición de memoria apuntada por el registro DI.
// }