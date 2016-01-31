#include <RTC.h>


//��������� ������������ �����
void RTC_init(void){

	// ������� LSI
	RCC->CSR |= RCC_CSR_LSION;
	// ���� ���������� LSI
	while ( !(RCC->CSR & RCC_CSR_LSIRDY) ){}

	// ������� ������������ ������ PWR
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	// ����� ������ �������� RTC ��������� � ���������� ������ 
	// ��� ���������� ������ ���������� ��������� ���� � PWR->CR
	PWR->CR |= PWR_CR_DBP;
	
  	// ������� �������� ������������ RTC( �� ���������������� ����������� ��������� LSI(40 kHz) )
	RCC->BDCR |= RCC_BDCR_RTCSEL_LSI;
	// ������� ����������� RTC
	RCC->BDCR |= RCC_BDCR_RTCEN;
	

	// �������� ������ �� ������
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	// ������ � ����� ��������������
	RTC->ISR |= RTC_ISR_INIT;
	// ���� ������������� ����� � ����� ��������������
	while( !(RTC->ISR & RTC_ISR_INITF) ){};	

	// ��������� ����������� ������������ �� 100(99+1).
	RTC->PRER = (uint32_t)(99 << 16);
	// ��������� ���������� ������������ �� 400(399+1).
	RTC->PRER |= (uint32_t)399;
	
	// ����� �� ������ ��������������
	RTC->ISR &= ~(RTC_ISR_INIT);
	
	// ������� ������ �� ������
	RTC->WPR = 0xFF;
}


// ��������� ������� � ���� ����� ��������� �������. ��� �������� � BCD.
void RTC_change (RTC_struct *value){
	uint32_t TR, DR;
	
	// �������� ������ �� ������
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

 	// ������ � ����� ��������������
	RTC->ISR |= RTC_ISR_INIT;
	
	// ���� ������������� ����� � ����� ��������������
	while( !(RTC->ISR & RTC_ISR_INITF) ){};	
	
	// ��������� �����
	TR = ( ((uint32_t)(value->hour) << 16) | ((uint32_t)(value->minute) << 8) | ((uint32_t)value->second) );
	RTC->TR = TR & RTC_TR_MASK;
	// ��������� ����
	DR = (uint32_t)(value->year) << 16  | (uint32_t)(value->week) << 13 | (uint32_t)(value->mount) << 8 | (uint32_t)(value->date);
	RTC->DR = DR & RTC_DR_MASK;
	
 	// ����� �� ������ ��������������
	RTC->ISR &= ~(RTC_ISR_INIT); 
	
	// ������� ������ �� ������
	RTC->WPR = 0xFF;
	
	RTC->BKP0R |= RTC_SET; // ������ ���������� ����
}

void RTC_IRQHandler(void){
	if(RTC->ISR & RTC_ISR_ALRAF){ // �������� Alarm
		
		GPIOA->ODR ^= GPIO_ODR_6;
		
		RTC->ISR &= ~(RTC_ISR_ALRAF);// ������� ���� ��������� ALARM
		EXTI->PR |= EXTI_PR_PR17; // ������� ���� ���������� ����� EXTI
	}
}

void RTC_alarm(RTC_struct *value, uint8_t msk){
	uint32_t alr, ms;
	/* ���������� ���������� */
	NVIC_EnableIRQ(RTC_IRQn);
	EXTI->IMR |= EXTI_IMR_MR17; // �������� ���������� 17 ������ EXTI
	EXTI->RTSR |= EXTI_RTSR_TR17; // ���������� �� ����������� ������
	
	// �������� ������ �� ������
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	
	RTC->CR |= RTC_CR_ALRAIE; // ���������� �� alarm. ������ ������ � ���������� ����.
	
	// ��������� ALARM
	RTC->CR &= ~(RTC_CR_ALRAE); 
	// ���� ������������� ����� � ����� ��������������
	while( !(RTC->ISR & RTC_ISR_ALRAWF) ){};
	
	RTC-> ALRMAR = ( (msk & 0x01) << 7 ); // MSK1
	RTC-> ALRMAR |= ( (msk & 0x02) << 14 ); // MSK2
	RTC-> ALRMAR |= ( (msk & 0x04) << 21 ); // MSK3
	RTC-> ALRMAR |= ( (msk & 0x08) << 28 ); // MSK4

	
	alr = ( (uint32_t)(value->date) << 24) | ((uint32_t)(value->hour) << 16) | ((uint32_t)(value->minute) << 8) | ((uint32_t)(value->second)); 
	// ���������� ����� ������������
	RTC->ALRMAR |= alr;
	
	// �������� ALARM
	RTC->CR |= RTC_CR_ALRAE;
	// ������� ������ �� ������
	RTC->WPR = 0xFF;
}

// ��������� ������������ � �������������� ��������� �����
void RTC_sets(RTC_struct *value){
		uint32_t TR, DR;
		// ������� LSI
		RCC->CSR |= RCC_CSR_LSION;
		// ���� ���������� LSI
		while ( !(RCC->CSR & RCC_CSR_LSIRDY) ){}
		
		// ������� ������������ ������ PWR
		RCC->APB1ENR |= RCC_APB1ENR_PWREN;
		// ����� ������ �������� RTC ��������� � ���������� ������ 
		// ��� ���������� ������ ���������� ��������� ���� � PWR->CR
		PWR->CR |= PWR_CR_DBP;
		
	if( !(RTC->ISR & RTC_ISR_INITS) ){ //���� �� ���������	

		// ������� �������� ������������ RTC( �� ���������������� ����������� ��������� LSI(40 kHz) )
		RCC->BDCR |= RCC_BDCR_RTCSEL_LSI;
		// ������� ����������� RTC
		RCC->BDCR |= RCC_BDCR_RTCEN;
	
		// �������� ������ �� ������
		RTC->WPR = 0xCA;
		RTC->WPR = 0x53;

		// ������ � ����� ��������������
		RTC->ISR |= RTC_ISR_INIT;
		// ���� ������������� ����� � ����� ��������������
		while( !(RTC->ISR & RTC_ISR_INITF) ){};	
		
		// ��������� ����������� ������������ �� 100(99+1).
		RTC->PRER = (uint32_t)(99 << 16);
		// ��������� ���������� ������������ �� 400(399+1).
		RTC->PRER |= (uint32_t)399;
		
		// ��������� �����
		TR = ( ((uint32_t)(value->hour) << 16) | ((uint32_t)(value->minute) << 8) | ((uint32_t)value->second) );
		RTC->TR = TR & RTC_TR_MASK;
		// ��������� ����
		DR = (uint32_t)(value->year) << 16  | (uint32_t)(value->week) << 13 | (uint32_t)(value->mount) << 8 | (uint32_t)(value->date);
		RTC->DR = DR & RTC_DR_MASK;
		
		// ����� �� ������ ��������������
		RTC->ISR &= ~(RTC_ISR_INIT); 

		
		// ������� ������ �� ������
		RTC->WPR = 0xFF;
		
	}

}


// ���������� �������� ����� ��������� �������. ��� �������� � BCD.
void RTC_get(RTC_struct * value){
	uint32_t TR, DR;
	TR = RTC->TR;
	DR = RTC->DR;
	
	// ������� �����
	value->hour = (uint8_t)(TR >> 16) & (uint8_t)0x3F; // ������� ������ ����� ���
	value->minute = (uint8_t)(TR >> 8) & (uint8_t)0x7F; // ������� ���� ���
	value->second = (uint8_t)(TR) & (uint8_t)0x7F;
	// ������� ����
	value->year = (uint8_t)(DR >> 16);
	value->week = (uint8_t)(DR >> 13) & (uint8_t)0x7;// ��� ����
	value->mount = (uint8_t)(DR >> 8) & (uint8_t)0x1F; // ���� ���
	value->date = (uint8_t)(DR) & (uint8_t)0x3F; // ����� ���
	
}

// ������� �� ��������� � BCD(����������).
uint8_t RTC_ByteToBcd2(uint8_t Value){
	uint8_t bcdhigh = 0;
	while (Value >= 10)	{
		bcdhigh++;
		Value -= 10;
	}
	return  ((uint8_t)(bcdhigh << 4) | Value);
}

// ������� �� BCD(����������) � �������� �����
uint8_t RTC_Bcd2ToByte(uint8_t Value){
	uint8_t tmp = 0;
	tmp = ((uint8_t)(Value & (uint8_t)0xF0) >> (uint8_t)0x4) * 10;
	return (tmp + (Value & (uint8_t)0x0F));
}

// ������� ������ ��������� BCD 
uint8_t RTC_Bcd_elder(uint8_t value){
	return (value >> 4);
}

// ������� ������ ��������� BCD
uint8_t RTC_Bcd_under(uint8_t value){
	return (value & (uint8_t)0x0F);
}