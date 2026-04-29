
t_registros registros;

void *memoria;
uint8_t memoria[MEM_SIZE];
void NOOP()
{
    registros +=1;
    
}


void SET(reg_id reg, uint32_t valor)
{
    switch (reg)
    {
        case AX: registros->AX = (uint8_t)valor; break;
        case BX: registros->BX = (uint8_t)valor; break;
        case CX: registros->CX = (uint8_t)valor; break;
        case DX: registros->DX = (uint8_t)valor; break;

        case PC:  registros->PC  = valor; break;  
        case EAX: registros->EAX = valor; break;
        case EBX: registros->EBX = valor; break;
        case ECX: registros->ECX = valor; break;
        case EDX: registros->EDX = valor; break;
        case SI:  registros->SI  = valor; break;
        case DI:  registros->DI  = valor; break;
    }
}

//GAGA
void MOV_IN (t_registro r, reg_id reg)
{
    uint32_t dir = r->SI;

    switch (reg)
    {
        // Registros de 8 bits
        case AX:
            r->AX = *((uint8_t*)memoria + dir);
            break;

        case BX:
            r->BX = *((uint8_t*)memoria + dir);
            break;

        case CX:
            r->CX = *((uint8_t*)memoria + dir);
            break;

        case DX:
            r->DX = *((uint8_t*)memoria + dir);
            break;

        // Registros de 32 bits (forma segura)
        case EAX:
            memcpy(&r->EAX, (uint8_t*)memoria + dir, sizeof(uint32_t));
            break;

        case EBX:
            memcpy(&r->EBX, (uint8_t*)memoria + dir, sizeof(uint32_t));
            break;

        case ECX:
            memcpy(&r->ECX, (uint8_t*)memoria + dir, sizeof(uint32_t));
            break;

        case EDX:
            memcpy(&r->EDX, (uint8_t*)memoria + dir, sizeof(uint32_t));
            break;
t_registros regist
        case SI:
            memcpy(&r->SI, (uint8_t*)memoria + dir, sizeof(uint32_t));
            break;

        case DI:
            memcpy(&r->DI, (uint8_t*)memoria + dir, sizeof(uint32_t));
            break;

        case PC:
            memcpy(&r->PC, (uint8_t*)memoria + dir, sizeof(uint32_t));
            break;
    }
}
//GAGA
void MOV_OUT()
{

}

//GAGA
void SUM(reg_id dest, reg_id orig)
{
    switch(dest)
    {
        switch (orig)
            {
                case AX: registros->AX += registros->AX; break;
                case BX: registros->AX += registros->BX; break;
                case CX: registros->AX += registros->CX; break;
                case DX: registros->AX += registros->DX; break;
                default: break;
            }
            break;

        case BX:
            switch (orig)
            {
                case AX: registros->BX += registros->AX; break;
                case BX: registros->BX += registros->BX; break;
                case CX: registros->BX += registros->CX; break;
                case DX: registros->BX += registros->DX; break;
                default: break;
            }
            break;

        // DESTINO 32 bits
        case EAX:
            switch (orig)
            {
                case EAX: registros->EAX += registros->EAX; break;
                case EBX: registros->EAX += registros->EBX; break;
                case ECX: registros->EAX += registros->ECX; break;
                case EDX: registros->EAX += registros->EDX; break;
                case SI:  registros->EAX += registros->SI;  break;
                case DI:  registros->EAX += registros->DI;  break;
                case PC:  registros->EAX += registros->PC;  break;
                default: break;
            }
            break;

        case EBX:
            switch (orig)
            {
                case EAX: registros->EBX += registros->EAX; break;
                case EBX: registros->EBX += registros->EBX; break;
                case ECX: registros->EBX += registros->ECX; break;
                case EDX: registros->EBX += registros->EDX; break;
                case SI:  registros->EBX += registros->SI;  break;
                case DI:  registros->EBX += registros->DI;  break;
                case PC:  registros->EBX += registros->PC;  break;
                default: break;
            }
            break;
    }
}

void SUB  (reg_id orig,reg_id dest)
{
    switch(dest)
    {
        switch (orig)
            {
                case AX: registros->AX -= registros->AX; break;
                case BX: registros->AX -= registros->BX; break;
                case CX: registros->AX -= registros->CX; break;
                case DX: registros->AX -= registros->DX; break;
                default: break;
            }
            break;

        case BX:
            switch (orig)
            {
                case AX: registros->BX -= registros->AX; break;
                case BX: registros->BX -= registros->BX; break;
                case CX: registros->BX -= registros->CX; break;
                case DX: registros->BX -= registros->DX; break;
                default: break;
            }
            break;

        // DESTINO 32 bits
        case EAX:
            switch (orig)
            {
                case EAX: registros->EAX -= registros->EAX; break;
                case EBX: registros->EAX -= registros->EBX; break;
                case ECX: registros->EAX -= registros->ECX; break;
                case EDX: registros->EAX -= registros->EDX; break;
                case SI:  registros->EAX -= registros->SI;  break;
                case DI:  registros->EAX -= registros->DI;  break;
                case PC:  registros->EAX -= registros->PC;  break;
                default: break;
            }
            break;

        case EBX:
            switch (orig)
            {
                case EAX: registros->EBX += registros->EAX; break;
                case EBX: registros->EBX += registros->EBX; break;
                case ECX: registros->EBX += registros->ECX; break;
                case EDX: registros->EBX += registros->EDX; break;
                case SI:  registros->EBX += registros->SI;  break;
                case DI:  registros->EBX += registros->DI;  break;
                case PC:  registros->EBX += registros->PC;  break;
                default: break;
            }
            break;
    }
}
void JNZ(reg_id reg, t_registros regist)
{

}

void COPY_MEM (reg_tam)
{
    
}