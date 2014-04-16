#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>

int main()
{
    sf::SoundBuffer BufferOne;
    sf::SoundBuffer BufferTwo;

    // load the files
    if (!BufferOne.LoadFromFile("Sounds/Silence.wav"))
    {
        std::cout << "Error Loading First Sound File." << std::endl;
        return 1;
    }

    if (!BufferTwo.LoadFromFile("Sounds/02.wav"))
    {
        std::cout << "Error Loading Second Sound File." << std::endl;
        return 1;
    }

    int iLength;
    int iShortLength;
    const sf::Int16* LongerSamples;
    const sf::Int16* ShorterSamples;

    if(BufferOne.GetSamplesCount() > BufferTwo.GetSamplesCount())
    {
        LongerSamples = BufferOne.GetSamples();
        ShorterSamples = BufferTwo.GetSamples();
        iLength = BufferOne.GetSamplesCount();
        iShortLength = BufferTwo.GetSamplesCount();
    }
    else
    {
        LongerSamples = BufferTwo.GetSamples();
        ShorterSamples = BufferOne.GetSamples();
        iLength = BufferTwo.GetSamplesCount();
        iShortLength = BufferOne.GetSamplesCount();
    }

    std::vector<sf::Int16> FinalSamplesVector;
    FinalSamplesVector.reserve(iLength);

    for(int i = 0; i < iLength; i++)
    {
        if(i < iShortLength)
        {
            double dSampleOne = (LongerSamples[i] + 32768.) / 65535.;
            double dSampleTwo = (ShorterSamples[i] + 32768.) / 65535.;
            double dResult = 0;

            if(dSampleOne < 0.5 && dSampleTwo < 0.5)
            {
                dResult = 2 * dSampleOne * dSampleTwo;
            }
            else
            {
                dResult = 2 * (dSampleOne + dSampleTwo) - 2 * dSampleOne * dSampleTwo - 1;
            }

            FinalSamplesVector.push_back(static_cast<sf::Int16>(dResult * 65535. - 32768.));
        }
        else
        {
            FinalSamplesVector.push_back(LongerSamples[i]);
        }
    }

    sf::SoundBuffer FinalBuffer;
    FinalBuffer.LoadFromSamples(&FinalSamplesVector[0], FinalSamplesVector.size(), 2, 44100);
    FinalBuffer.SaveToFile("output.wav");

    sf::Sound FinalSound;
    FinalSound.SetBuffer(FinalBuffer);
    FinalSound.Play();

    // this is dirty!!!
    while(1)
    {
        if(FinalSound.GetStatus() == sf::Sound::Stopped)
            return 0;
    }
}
