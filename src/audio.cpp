#include "audio.h"

void playWav()
{
    if (!audioFile)
        return;

    int bytesRead = audioFile.read(chunkBuffer, CHUNK_SIZE);

    if (bytesRead > 0)
    {
        I2S.write(chunkBuffer, (size_t)bytesRead);
    }

    if (bytesRead < CHUNK_SIZE)
    {
        // printf("End of file. Restarting...\n");
        audioFile.seek(44);
    }
}