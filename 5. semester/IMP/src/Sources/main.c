// Name  : IMP - Aplikácia demonštrujúca možnosti WDOG na Kinetis K60
// Author: Martin Fekete <xfekete00@stud.fit.vutbr.cz>
// Date  : 29.10.2020

#include <stdio.h>
#include <string.h>
#include "MK60D10.h"

// Macros for bit-level registers manipulation
#define GPIO_PIN_MASK 0x1Fu
#define GPIO_PIN(x) (((1)<<(x & GPIO_PIN_MASK)))

// Mapping of LEDs and buttons to specific port pins
// LED_D10 == cycle
// LED_D11 == reset
#define LED_D10 0x10      // Port B, bit 4
#define LED_D11 0x8       // Port B, bit 3

// BTN_SW4 == refresh WDOG
// BTN_SW5 == print stats about WDOG
#define BTN_SW4 0x8000000 // Port E, bit 27
#define BTN_SW6 0x800     // Port E, bit 11

// ----------- VARIABLES -----------

// For reset sources
static RCM_Type *rcm = RCM;

// Buttons for refresh and printing
int pressed_sw4 = 0;
int pressed_sw6 = 0;

// Beepping flag
int beep_flag = 0;

// Periodic or window mode
// 0b0101 == WDOG enabled, CLK source LPO, debug, periodic mode
// 0b1101 == WDOG enabled, CLK source LPO, debug, windowing mode
int mode[2] = {
    0b0101,
    0b1101
};

// Period size
// ~5s, ~10s respectively
int tovalh[2] = {
    0,
    0
};
int tovall[2] = {
    1000,
    2000
};

// Window size
// Note: be always lower than the period size
// ~5s, ~10s respectively
int winh[2] = {
    0,
    0
};
int winl[2] = {
    500,
    1000
};

// ----------- FUNCTIONS -----------

// Delay for beeping
void delay(long long bound)
{
  for (long long i = 0; i < bound; i++);
}

// Speaker beeping
void beep()
{
    for (unsigned int q = 0; q < 500; q++)
    {
        PTA->PDOR = GPIO_PDOR_PDO(0x10);
        delay(500);
        PTA->PDOR = GPIO_PDOR_PDO(0x00);
        delay(500);
    }
}


// Returns reset source form SRS register
uint32_t getResetSRC()
{
	return (uint32_t)((uint32_t) rcm -> SRS0 | ((uint32_t) rcm -> SRS1 << 8U));
}


// Prints WDOG stats
// Reset count, reset source, WDOG timer, WDOG window if enabled
void printStats()
{
	uint32_t rstsrc = getResetSRC();

    printf("--------------------\n");
    printf("Reset no: %d", WDOG_RSTCNT);
    printf("Reset sources:\n");
    printf("--------------------\n");

	if (rstsrc & 0x1U)
		printf("\r\n--- Reset: Low Leakage Wakeup Reset ---\r\n");
	else if (rstsrc &  0x2U)
		printf("\r\n--- Reset: Low-Voltage Detect Reset ---\r\n");
	else if (rstsrc &  0x4U)
		printf("\r\n--- Reset: Loss-of-Clock Reset ---\r\n");
	else if (rstsrc &  0x8U)
		printf("\r\n--- Reset: Loss-of-Lock Reset ---\r\n");
	else if (rstsrc & 0x20U)
		printf("\r\n--- Reset: Watchdog ---\r\n");
	else if (rstsrc & 0x40U)
		printf("\r\n--- Reset: External Reset Pin ---\r\n");
	else if (rstsrc & 0x80U)
		printf("\r\n--- Reset: Power-On Reset ---\r\n");
	else if (rstsrc & (0x1U << 8U))
		printf("\r\n--- Reset: JTAG Generated Reset ---\r\n");
	else if (rstsrc & (0x2U << 8U))
		printf("\r\n--- Reset: Core Lockup ---\r\n");
	else if (rstsrc & (0x4U << 8U))
		printf("\r\n--- Reset: Software ---\r\n");
	else if (rstsrc & (0x8U << 8U))
		printf("\r\n--- Reset: MDM-AP System Reset Request ---\r\n");
	else if (rstsrc & (0x10U << 8U))
		printf("\r\n--- Reset: EzPort Reset ---\r\n");
	else if (rstsrc & (0x20U << 8U))
		printf("\r\n--- Reset: Stop Mode Acknowledge Error Reset ---\r\n");
	else if (rstsrc & 0xffffffffU)
		printf("\r\n--- Reset: External Reset Pin ---\r\n");


	printf("--------------------\n");
	printf("WDOG_TMROUTL: %d\n", WDOG_TMROUTL);

	// Windowing mode
	if (WDOG_STCTRLH & 0b1000 != 0)
	{
		printf("WDOG_WINL: %d\n", WDOG_WINL);
	}

	printf("--------------------\n");
	printf("To reset microcontroller do not press anything\n");

}


// ----------- INIT -----------

// Initialize the MCU - basic clock settings
void MCUInit(void)
{
    MCG_C4 |= (MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x01));
    SIM_CLKDIV1 |= SIM_CLKDIV1_OUTDIV1(0x00);
}

void PortsInit(void)
{
    // Turn on all port clocks
    SIM->SCGC5 = SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK | SIM_SCGC5_PORTA_MASK;

    // Set corresponding PTB pins (connected to LED's) for GPIO functionality
    PORTB->PCR[5] = PORT_PCR_MUX(0x01); // D9
    PORTB->PCR[4] = PORT_PCR_MUX(0x01); // D10
    PORTB->PCR[3] = PORT_PCR_MUX(0x01); // D11
    PORTB->PCR[2] = PORT_PCR_MUX(0x01); // D12

    PORTE->PCR[10] = PORT_PCR_MUX(0x01); // SW2
    PORTE->PCR[12] = PORT_PCR_MUX(0x01); // SW3
    PORTE->PCR[27] = PORT_PCR_MUX(0x01); // SW4
    PORTE->PCR[26] = PORT_PCR_MUX(0x01); // SW5
    PORTE->PCR[11] = PORT_PCR_MUX(0x01); // SW6

    PORTA->PCR[4] = PORT_PCR_MUX(0x01);  // Speaker

    // Change corresponding PTB port pins as outputs
    PTA->PDDR =  GPIO_PDDR_PDD(0x10);
    PTB->PDDR = GPIO_PDDR_PDD(0x3C);     // LED ports as outputs
    PTB->PDOR |= GPIO_PDOR_PDO(0x3C);    // Turn all LEDs OFF
}

void LPTMR0_IRQHandler(void)
{
    LPTMR0_CSR |=  LPTMR_CSR_TCF_MASK;   // Writing 1 to TCF tclear the flag
    GPIOB_PDOR ^= LED_D10;               // Invert D10 state
    GPIOB_PDOR ^= LED_D11;               // Invert D11 state
    beep_flag = !beep_flag;              // Invert beeping flag
}

void LPTMR0Init()
{
    SIM_SCGC5 |= SIM_SCGC5_LPTIMER_MASK; // Enable clock to LPTMR
    LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK;   // Turn OFF LPTMR to perform setup

    LPTMR0_PSR = ( LPTMR_PSR_PRESCALE(0) // 0000 is div 2
                 | LPTMR_PSR_PBYP_MASK   // LPO feeds directly to LPT
                 | LPTMR_PSR_PCS(1)) ;   // Use the choice of clock

    LPTMR0_CMR = 0x200;                  // Set compare value

    LPTMR0_CSR = ( LPTMR_CSR_TCF_MASK    // Clear any pending interrupt (now)
                 | LPTMR_CSR_TIE_MASK);  // LPT interrupt enabled

    NVIC_EnableIRQ(LPTMR0_IRQn);         // Enable interrupts from LPTMR0
    NVIC_EnableIRQ(WDOG_EWM_IRQn);       // Enable interrupts from WDOG

    LPTMR0_CSR |= LPTMR_CSR_TEN_MASK;    // Turn ON LPTMR0 and start counting
}


void Init()
{
	MCUInit();
	PortsInit();
	LPTMR0Init();
}

// ----------- STATS -----------

int main(void)
{
    printf("Demo start\n");
    printf("----------\n");

    // If reset was not due to the watchdog, clear the WDOG_RSTCNT counter
    if (!(getResetSRC() & 32)) {
        WDOG_RSTCNT |= UINT16_MAX;
    }

    // --------------------------
    // Periodic mode
    // --------------------------
    if (WDOG_RSTCNT == 0 || WDOG_RSTCNT == 1)
    {
        printf("\n1. Periodic mode\n");

        WDOG_UNLOCK = WDOG_UNLOCK_WDOGUNLOCK(0xC520);
        WDOG_UNLOCK = WDOG_UNLOCK_WDOGUNLOCK(0xD928);

        // Periodic mode
        WDOG_STCTRLH = mode[0];

        // WDOG timeout
        WDOG_TOVALH = WDOG_TOVALH_TOVALHIGH(tovalh[WDOG_RSTCNT]);
        WDOG_TOVALL = WDOG_TOVALL_TOVALLOW(tovall[WDOG_RSTCNT]);

        Init();

        // Let the user know
        beep();

        while (1)
        {
            // If pressed button SW6, print stats
            if (!pressed_sw6 && !(GPIOE_PDIR & BTN_SW6))
            {
            	pressed_sw6 = 1;
            	printStats();
            }
            else if (GPIOE_PDIR & BTN_SW6)
            {
            	pressed_sw6 = 0;
            }

            // If pressed button SW4, refresh WDOG
            if (!pressed_sw4 && !(GPIOE_PDIR & BTN_SW4))
            {
            	pressed_sw4 = 1;

                WDOG_REFRESH = WDOG_REFRESH_WDOGREFRESH(0xA602);
                WDOG_REFRESH = WDOG_REFRESH_WDOGREFRESH(0xB480);

                printf("WDOG refreshed!\n");
            }
            else if (GPIOE_PDIR & BTN_SW4)
            {
            	pressed_sw4 = 0;
            }

            // Waiting for timeout to reset the system
        }
    }

    // --------------------------
    // Windowing mode
    // --------------------------
    if (WDOG_RSTCNT == 2 || WDOG_RSTCNT == 3)
    {
    	beep();
    	delay(500);
    	beep();
        printf("\n2. Windowing mode\n");

        WDOG_UNLOCK = WDOG_UNLOCK_WDOGUNLOCK(0xC520);
        WDOG_UNLOCK = WDOG_UNLOCK_WDOGUNLOCK(0xD928);

        // Windowing mode
        WDOG_STCTRLH = mode[1];

        // WDOG timeout
        WDOG_TOVALH = WDOG_TOVALH_TOVALHIGH(tovalh[WDOG_RSTCNT-2]);
        WDOG_TOVALL = WDOG_TOVALL_TOVALLOW(tovall[WDOG_RSTCNT-2]);

        // WDOG window size
        WDOG_WINH = WDOG_WINH_WINHIGH(winh[WDOG_RSTCNT-2]);
        WDOG_WINL = WDOG_WINL_WINLOW(winl[WDOG_RSTCNT-2]);

        Init();

        // Let the user know
        beep();
        delay(500);
        beep();

        while (1)
        {
            // If pressed button SW6, print stats
        	if (!pressed_sw6 && !(GPIOE_PDIR & BTN_SW6))
            {
        		pressed_sw6 = 1;
        		printStats();
            }
            else if (GPIOE_PDIR & BTN_SW6)
            {
            	pressed_sw6 = 0;
            }

            // If pressed button SW4, refresh WDOG
        	if (!pressed_sw4 && !(GPIOE_PDIR & BTN_SW4))
            {
        		pressed_sw4 = 1;

                WDOG_REFRESH = WDOG_REFRESH_WDOGREFRESH(0xA602);
                WDOG_REFRESH = WDOG_REFRESH_WDOGREFRESH(0xB480);

                printf("WDOG refreshed!\n");
            }
            else if (GPIOE_PDIR & BTN_SW4)
            {
            	pressed_sw4 = 0;
            }

        	// Waiting for timeout to reset the system
        }
    }

    printf("\n----------\n");
    printf("Demo end!\n");

    // Never end the program
    while (1);
    return 0;
}
