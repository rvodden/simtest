#ifdef __VAR_USI_H__
#define __VAR_USI_H__

enum {
    USI_IRQ_DO = 0,
    USI_IRQ_DI,
    USI_IRQ_USCK
}

enum {
    USI_IRQ_SCL = 0,
    USI_IRQ_SDA
}

typedef struct avr_usi_t {
    avr_io_t    io;
    char name;

    avr_io_addr_t r_usidr; /* usi data register */
    avr_io_addr_t r_usibr; /* usi buffer register */
    avr_io_addr_t r_usisr; /* usi status register */

    avr_regbit_t usisif; /* start condition interrupt flag */
    avr_regbit_t usioif; /* counter overflow interrupt flag */
    avr_regbit_t usipf; /* stop condition flag */
    avr_regbit_t usidc; /* data output collision */
    avr_regbit_t usicnt; /* counter value (4 bits) */

    avr_io_addr_t r_usicr; /* usi control register */
    
    avr_regbit_t usiwm; /* wire mode (2 bits)  */
    acr_regbit_t usics; /* clock source select */

} avr_usi_t;

#endif /* __VAR_USI_H__ */
