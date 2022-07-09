#include <pic18f4520.h>

/*
 * File:   main.c
 * Author: Jessica
 *
 * Created on 11 de junho de 2022, 18:59
 */

// diretiva de configuração: pino reset externo, watchdog desabilitado, oscilador High Speed
#include <xc.h>
#pragma config  OSC = HS, WDT = OFF,BOREN = OFF

//declara as variáveis necessariamente globais
unsigned char verde[3][3], vermelho[3][3], cursor[3][3], linha, conta, cor;
unsigned int pisca;

#define _XTAL_FREQ 16000000
#define SEL PORTBbits.RB4
#define MOVE PORTBbits.RB3
#define RESET PORTBbits.RB5

unsigned char cont = 0;

void __interrupt() interrupt_PIC()
{
    
       
    //confere se a interrupção é devido a overflow no Timer0
    if(INTCONbits.TMR0IF == 1)
    {
        //zera o PORTB pra evitar "sombras" nos LEDs
        PORTD = 0x00;
        //ativa o pino do PORTA correspondente a cada linha
        if(linha == 0){
            PORTBbits.RB0 = 0x00;
            PORTBbits.RB1 = 0x01;
            PORTBbits.RB2 = 0x01;
        }
        else if(linha == 1){
            PORTBbits.RB0 = 0x01;
            PORTBbits.RB1 = 0x00;
            PORTBbits.RB2 = 0x01;
        }
        else if(linha == 2){
            PORTBbits.RB0 = 0x01;
            PORTBbits.RB1 = 0x01;
            PORTBbits.RB2 = 0x00;
        }
 
        //Lógica para acender cada LED
        PORTDbits.RD0 = vermelho[linha][0] | (cursor[linha][0] & pisca & cor);
        PORTDbits.RD1 = vermelho[linha][1] | (cursor[linha][1] & pisca & cor);
        PORTDbits.RD2 = vermelho[linha][2] | (cursor[linha][2] & pisca & cor);
        PORTDbits.RD3 = verde[linha][0] | (cursor[linha][0] & pisca & !cor);
        PORTDbits.RD4 = verde[linha][1] | (cursor[linha][1] & pisca & !cor);
        PORTDbits.RD5 = verde[linha][2] | (cursor[linha][2] & pisca & !cor);
         
        //incrementa a variável linha para que na próxima interrupção a 
        //próxima linha seja ativada
        linha++;
        if(linha == 3)
            linha = 0;
         
        //incrementa a variável conta para contar o tempo e fazer o
        //cursor piscar.
        conta++;
        if(conta == 100)
        {
            conta = 0;
            pisca = !pisca;
        }
        //reseta os parâmetros do Timer0 para a interrupção ocorrer novamente
        TMR0H = 0;         
        TMR0L = 0;
        INTCONbits.TMR0IF = 0;
    }
}

void conf_interrupt(void){
INTCONbits.INT0IF = 0; // Apaga flag indigador de interrupção
INTCONbits.INT0IE = 1; // Habilita int externa 0

T0CONbits.T08BIT = 1; //TMR0 operando com 16 bits
T0CONbits.T0CS   = 0; //seleciona clock intereno
T0CONbits.PSA    = 0; //com prescaler
T0CONbits.T0PS   = 100; //dividindo por 8
T0CONbits.TMR0ON = 1; // Timer 0 ligado

ADCON1 = 0x0F;
}

//rotina para verificar de vitórias
unsigned char testa_ganhou(unsigned char teste[3][3])
{
    //verifica se existe alguma coluna completa
    if(teste[0][0] && teste[1][0] && teste[2][0])
        return 1;
    if(teste[0][1] && teste[1][1] && teste[2][1])
        return 1;
    if(teste[0][2] && teste[1][2] && teste[2][2])
        return 1;
     
    //verifica se existe alguma linha completa
    if(teste[0][0] && teste[0][1] && teste[0][2])
        return 1;
    if(teste[1][0] && teste[1][1] && teste[1][2])
        return 1;
    if(teste[2][0] && teste[2][1] && teste[2][2])
        return 1;
     
    //verifica as duas diagonais
    if(teste[0][0] && teste[1][1] && teste[2][2])
        return 1;
    if(teste[0][2] && teste[1][1] && teste[2][0])
        return 1;   
     
    //retorna zero se não há combinação válida
    return 0;
}
 
//rotina para verificar se "deu velha"
unsigned char testa_velha(unsigned char teste1[3][3], unsigned char teste2[3][3])
{
    unsigned char i, j, soma;
     
    soma = 0;
     
    //soma o valor de todas as posições das duas matrizes e se todas as posições
    // estiverem completas o valor da soma sera 9, assim indicando que deu velha.
    for(i = 0; i < 3; i++)
    {
        for(j = 0; j < 3; j++)
            soma += teste1[i][j] + teste2[i][j];
    }
     
    if(soma == 9)
        return 1;
    else
        return 0;
}

//zera todas as posições na matriz
void limpa_matriz(unsigned char matriz[3][3])
{
    unsigned char i, j;
     
    for(i = 0; i < 3; i++)
    {
        for(j = 0; j < 3; j++)
            matriz[i][j] = 0;
    }
}
 
void main(void)
{
    //cria as variáveis locais
    unsigned char i, j, MOVE_Old, SEL_Old, pontuacao_vermelho, pontuacao_verde, marcados[3][3], aux;
    signed char posicao;
    conf_interrupt();
            
    //inicializa as matrizes e variáveis
    limpa_matriz(verde);
    limpa_matriz(vermelho);
    limpa_matriz(cursor);
    limpa_matriz(marcados);
     
    pisca = 1;
    linha = 0;
    posicao = -1;
    MOVE_Old = 1;
    SEL_Old = 1;
    cursor[0][0] = 1;
    cor = 0;
    aux = 0;
    pontuacao_vermelho = 0;
    pontuacao_verde = 0;

     
    //configura o PIC
    CMCON = 0xFF;
    TRISB = 0x18;
    TRISD = 0x00;
    PORTB = 1;
    PORTD = 0;
    TMR0H = 11;         
    INTCON = 0xA0;
     
    while(1)
    {
        
        //testa se o botão que move o cursor está pressionado
        if(MOVE == 1 && MOVE_Old == 1)
        {    
            aux = 0;
            cursor[(posicao)/3][(posicao)%3] = 0;
            
            posicao++;
            while(aux != 1) {
                if((posicao) == 9)
                    posicao = 0;
                if(marcados[(posicao)/3][(posicao)%3] == 1) { 
                    posicao++;
                } else {
                    aux = 1;
                    cursor[(posicao)/3][(posicao)%3] = 1;
                    MOVE_Old = 0;
                } 
                
                
            }
            cursor[(posicao)/3][(posicao)%3] = 1;
            MOVE_Old = 0;    
        }
        //verifica se o jogador soltou o botão
        if(MOVE == 0 && MOVE_Old == 0)
            MOVE_Old = 1;
         
        //Marca na matriz correspondente a posição da jogada
        if(SEL == 1 && SEL_Old == 1)
        {
            
            if(cor == 0 && vermelho[(posicao)/3][(posicao)%3] == 0 && verde[(posicao)/3][(posicao)%3] == 0)
            {
                verde[(posicao)/3][(posicao)%3] = 1;
                marcados[(posicao)/3][(posicao)%3] = 1;
                cor = 1;
            }
        
            
            else if(cor == 1 && verde[(posicao)/3][(posicao)%3] == 0 && vermelho[(posicao)/3][(posicao)%3] == 0 )
            {
                vermelho[(posicao)/3][(posicao)%3] = 1;
                marcados[(posicao)/3][(posicao)%3] = 1;
                cor = 0;
            }
             
            aux = 0;
            cursor[(posicao)/3][(posicao)%3] = 0;
            
            posicao++;
            while(aux != 1) {
                if((posicao) == 9)
                    posicao = 0;
                if(marcados[(posicao)/3][(posicao)%3] == 1) { 
                    posicao++;
                } else {
                    aux = 1;
                    cursor[(posicao)/3][(posicao)%3] = 1;
                } 
                
                
            }
            cursor[(posicao)/3][(posicao)%3] = 1;     
            SEL_Old = 0;
        }
        //verifica se o jogador soltou o botão
        if(SEL == 0 && SEL_Old == 0) 
            SEL_Old = 1;      
        
        if(testa_ganhou(verde))
        {
            limpa_matriz(vermelho);
            limpa_matriz(verde);
            limpa_matriz(cursor);
            limpa_matriz(marcados);
            pontuacao_verde++;
            cor = 0;
             
            for(i = 0; i < pontuacao_verde; i++)
            {
                verde[i/3][i%3] = 1;
                __delay_ms(200);
            }
            __delay_ms(1000);
             
            if(pontuacao_verde == 9)
            {
                for(i = pontuacao_verde+1; i > 0; i--)
                {
                    verde[(i-1)/3][(i-1)%3] = 0;
                    __delay_ms(100);
                }
                pontuacao_vermelho = 0;
                pontuacao_verde = 0;   
            }
            __delay_ms(500);
             
            limpa_matriz(verde);
            linha = 0;
            posicao = 0;
            cursor[0][0] = 1;
            cor = 1;
        }
        //confere se o jogador vermelho ganhou
        if(testa_ganhou(vermelho))
        {
            limpa_matriz(vermelho);
            limpa_matriz(verde);
            limpa_matriz(cursor);
            limpa_matriz(marcados);
            pontuacao_vermelho++;
            cor = 1;
             
            for(i = 0; i < pontuacao_vermelho; i++)
            {
                vermelho[i/3][i%3] = 1;
                __delay_ms(200);
            }
            __delay_ms(1000);
             
            if(pontuacao_vermelho == 9)
            {
                for(i = pontuacao_vermelho+1; i > 0; i--)
                {
                    vermelho[(i-1)/3][(i-1)%3] = 0;
                    __delay_ms(100);
                }
                pontuacao_vermelho = 0;
                pontuacao_verde = 0;  
            }
            __delay_ms(500);
             
            limpa_matriz(vermelho);
            linha = 0;
            posicao = 0;
            cursor[0][0] = 1;
            cor = 0;
        }
        //confere se deu velha
        if(testa_velha(verde, vermelho))
        {
            limpa_matriz(verde);
            limpa_matriz(vermelho);
            limpa_matriz(cursor);
            limpa_matriz(marcados);
            __delay_ms(1000);
            linha = 0;
            posicao = 0;
            cursor[0][0] = 1;
            cor = 0;            
        }
     
    }
}