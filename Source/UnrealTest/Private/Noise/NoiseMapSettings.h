#pragma once

#include "CoreMinimal.h"
#include "NoiseMapSettings.generated.h"

USTRUCT(BlueprintType)
struct FNoiseMapSettings {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map", meta = (ClampMin = "-200", ClampMax = "200", SliderMin = "-200", SliderMax = "200"))
    int Octaves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map", meta = (ClampMin = "-200", ClampMax = "200", SliderMin = "-200", SliderMax = "200"))
    float Frequencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map", meta = (ClampMin = "-200", ClampMax = "200", SliderMin = "-200", SliderMax = "200"))
    TArray<int> Amplitudes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map", meta = (ClampMin = "-200", ClampMax = "200", SliderMin = "-200", SliderMax = "200"))
    float Lacunarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map", meta = (ClampMin = "-200", ClampMax = "200", SliderMin = "-200", SliderMax = "200"))
    float Gain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map", meta = (ClampMin = "-200", ClampMax = "200", SliderMin = "-200", SliderMax = "200"))
    float WeightedStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map", meta = (ClampMin = "-200", ClampMax = "200", SliderMin = "-200", SliderMax = "200"))
    int DomainWarpAmp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map", meta = (ClampMin = "-200", ClampMax = "200", SliderMin = "-200", SliderMax = "200"))
    float DomainWarpFrequencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map", meta = (ClampMin = "-200", ClampMax = "200", SliderMin = "-200", SliderMax = "200"))
    int DomainWarpOctaves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map", meta = (ClampMin = "-200", ClampMax = "200", SliderMin = "-200", SliderMax = "200"))
    float DomainWarpLacunarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Map", meta = (ClampMin = "-200", ClampMax = "200", SliderMin = "-200", SliderMax = "200"))
    float DomainWarpGain;
};
