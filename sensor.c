// file input.c
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

#define INCREASE_INPUT_INDEX 0
#define DECREASE_INPUT_INDEX 1
#define DISPLAY_INPUT_INDEX 2
#define RESET_INPUT_INDEX 3

#define INCREASE_INPUT_OFFSET 25
#define DECREASE_INPUT_OFFSET 10
#define DISPLAY_INPUT_OFFSET 17
#define RESET_INPUT_OFFSET 18

#define ZERO_OUTPUT_OFFSET 27
#define ONE_OUTPUT_OFFSET 23
#define TWO_OUTPUT_OFFSET 22
#define THREE_OUTPUT_OFFSET 24

#define INPUT_LINES_NUMBER 4
#define OUTPUT_LINES_NUMBER 4

#define TIMEOUT_SECONDS 60
#define CONSUMER "counter"

int initialize_output_lines(struct gpiod_chip *chip, struct gpiod_line_bulk* output_lines);
int initialize_input_lines(struct gpiod_chip *chip, struct gpiod_line_bulk* input_lines);
int display_counter(int counter, struct gpiod_line_bulk* output_lines);

int main(int argc, char *argv[])
{
    printf("abc\n");
    struct gpiod_chip *chip;
    struct gpiod_line_bulk input_lines;
    struct gpiod_line_bulk output_lines;
    struct gpiod_line_bulk events;
    struct gpiod_line_event event;

    int err;
    struct timespec timeout;
    int counter = 0;

    chip = gpiod_chip_open("/dev/gpiochip0");
    if (!chip)
    {
        perror("gpiod_chip_open");
        goto cleanup;
    }

    printf("chip\n");

    gpiod_line_bulk_init(&output_lines);
    err = initialize_output_lines(chip, &output_lines);
    if (err)
    {
        goto cleanup;
    }
    
    gpiod_line_bulk_init(&input_lines);
    err = initialize_input_lines(chip, &input_lines);
    if (err)
    {
        goto cleanup;
    }

    printf("start for\n");

    
    for (;;)
    {
        timeout.tv_sec = TIMEOUT_SECONDS;
        timeout.tv_nsec = 0;
        gpiod_line_bulk_init(&events);
        err = gpiod_line_event_wait_bulk(&input_lines, &timeout, &events);
        if (err == -1)
        {
            perror("gpiod_line_event_wait_bulk");
            goto cleanup;
        }
        else if (err == 0)
        {
            fprintf(stderr, "wait timed out\n");
            break;
        }

        for (int i = 0; i < gpiod_line_bulk_num_lines(&events); i++)
        {
            struct gpiod_line *line;
            line = gpiod_line_bulk_get_line(&events, i);
            if (!line)
            {
                fprintf(stderr, "unable to get line %d\n", i);
                continue;
            }
            
            err = gpiod_line_event_read(line, &event);
            if (err)
            {
                perror("gpiod_line_event_read");
                goto cleanup;
            }

            switch (gpiod_line_offset(line))
            {
                case INCREASE_INPUT_OFFSET:
                    counter = (counter + 1) % (1 << OUTPUT_LINES_NUMBER);
                    break;
                
                case DECREASE_INPUT_OFFSET:
                    counter = (counter - 1) % (1 << OUTPUT_LINES_NUMBER);
                    break;

                case DISPLAY_INPUT_OFFSET:
                    display_counter(counter, &output_lines);
                    break;
                
                case RESET_INPUT_OFFSET:
                    counter = 0;
                    err = display_counter(0, &output_lines);
                    if (err)
                    {
                        goto cleanup;
                    }
                    break;

                default:
                    break;
            }
        }
    }

cleanup:
    gpiod_line_release_bulk(&input_lines);
    gpiod_line_release_bulk(&output_lines);
    gpiod_chip_close(chip);

    return EXIT_SUCCESS;
}

int initialize_output_lines(struct gpiod_chip *chip, struct gpiod_line_bulk* output_lines)
{
    unsigned int output_offsets[OUTPUT_LINES_NUMBER];
    int output_default_vals[OUTPUT_LINES_NUMBER];
    int err;

    output_offsets[0] = ZERO_OUTPUT_OFFSET;
    output_offsets[1] = ONE_OUTPUT_OFFSET;
    output_offsets[2] = TWO_OUTPUT_OFFSET;
    output_offsets[3] = THREE_OUTPUT_OFFSET;
    printf("initialize output 1\n");
    err = gpiod_chip_get_lines(chip, output_offsets, OUTPUT_LINES_NUMBER, output_lines);
    if (err)
    {
        perror("gpiod_chip_get_lines");
        return -1;
    }

    for (int i = 0; i < OUTPUT_LINES_NUMBER; i++)
    {
        output_default_vals[i] = 0;
    }

    err = gpiod_line_request_bulk_output(output_lines, CONSUMER, output_default_vals);
    if (err)
    {
        perror("gpiod_line_request_bulk_output");
        return -1;
    }
    printf("initialize output 2\n");
    return 0;
}

int initialize_input_lines(struct gpiod_chip *chip, struct gpiod_line_bulk* input_lines)
{
    unsigned int input_offsets[INPUT_LINES_NUMBER];
    int values[INPUT_LINES_NUMBER];
    int err;

    input_offsets[INCREASE_INPUT_INDEX] = INCREASE_INPUT_OFFSET;
    values[INCREASE_INPUT_INDEX] = -1;

    input_offsets[DECREASE_INPUT_INDEX] = DECREASE_INPUT_OFFSET;
    values[DECREASE_INPUT_INDEX] = -1;

    input_offsets[DISPLAY_INPUT_INDEX] = DISPLAY_INPUT_OFFSET;
    values[DISPLAY_INPUT_INDEX] = -1;

    input_offsets[RESET_INPUT_INDEX] = RESET_INPUT_OFFSET;
    values[RESET_INPUT_INDEX] = -1;

    printf("initialize input 1\n");

    err = gpiod_chip_get_lines(chip, input_offsets, INPUT_LINES_NUMBER, input_lines);
    if (err)
    {
        perror("gpiod_chip_get_lines");
        return -1;
    }

    printf("initialize input 2\n");

    err = gpiod_line_request_bulk_rising_edge_events(input_lines, CONSUMER);
    if (err)
    {
        perror("gpiod_line_request_bulk_rising_edge_events");
        return -1;
    }
    printf("initialize input 3\n");

    return 0;
}

int display_counter(int counter, struct gpiod_line_bulk *output_lines)
{
    int values[OUTPUT_LINES_NUMBER];
    int err;

    for (int i = 0; i < OUTPUT_LINES_NUMBER; i++)
    {
        values[i] = (counter & (1 << i)) >> i;
    }

    err = gpiod_line_set_value_bulk(output_lines, values);
    if (err)
    {
        perror("gpiod_line_get_value_bulk");
        return -1;
    }

    return 0;
}