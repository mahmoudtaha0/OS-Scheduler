void write_to_file(const char *filename, const char *params, ...)
{
    FILE *file = fopen(filename, "a");

    if (file == NULL)
    {
        perror("Error opening file for append");
        return;
    }

    // using variadic functions...
    va_list args;
    va_start(args, params);

    // write the statistics
    vfprintf(file, params, args);

    va_end(args);

    fclose(file);
}