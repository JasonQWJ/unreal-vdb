// Copyright 2022 Eidos-Montreal / Eidos-Sherbrooke

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once 

#include "CoreMinimal.h"
#include "VdbComponentBase.h"

#include "VdbResearchComponent.generated.h"

class UVdbSequenceComponent;

// If you do not care about Unreal features integration, I recommend using this "Research" component.
// It allows you to experiment with OpenVDB / NanoVDB rendering, without having to worry 
// about most Unreal compatibilities. 
// 
// These NanoVDB are rendered at the end of the graphics pipeline, just before the Post Processes. 
// 
// This cannot be used in production, this is only used for research and experimentation purposes. It will 
// probably be incompatible with a lot of other Unreal features (but we don't care).
// Also note that this component can hack into the Unreal's path-tracer, and render high quality images.
// We made the delibarate choice to only handle NanoVDB FogVolumes with this component, because they benefit most 
// from experimentation and path-tracers, and are still an active research area (offline and realtime).
UCLASS(Blueprintable, ClassGroup = Rendering, hideCategories = (Activation, Collision, Cooking, HLOD, Navigation, VirtualTexture), meta = (BlueprintSpawnableComponent))
class UVdbResearchComponent : public UVdbComponentBase
{
	GENERATED_UCLASS_BODY()

	virtual ~UVdbResearchComponent();

	//----------------------------------------------------------------------------
	// Volume Attributes
	//----------------------------------------------------------------------------

	// Mandatory density volume. Only supports NanoVDB FogVolumes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Volumetric)
	TObjectPtr<class UVdbVolumeBase> VdbDensity;

	// Optional temperature volume. Only supports NanoVDB FogVolumes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Volumetric)
	TObjectPtr<class UVdbVolumeBase> VdbTemperature;

	// Max number of ray bounces
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Volumetric, meta = (ClampMin = "1", UIMin = "1", ClampMax = "50", UIMax = "20"))
	int32 MaxRayDepth = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Volumetric, meta = (ClampMin = "1", UIMin = "1"))
	int32 SamplesPerPixel = 1;

	//-------------------------------------------------------------------------------
	//    Principled Volume Shader Options, inspired by these two sources:
	// https://docs.arnoldrenderer.com/display/A5AFMUG/Standard+Volume#StandardVolume-Displacement
	// https://docs.blender.org/manual/en/latest/render/shader_nodes/shader/volume_principled.html
	//----------------------------------------------------------------------------

	// Volume scattering color. This acts as a multiplier on the scatter color, to texture the color of the volume.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Principled Volume")
	FLinearColor Color = FLinearColor(0.5, 0.5, 0.5, 1.0);

	// Density multiplier of the volume, modulating VdbDensity values 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Principled Volume", meta = (ClampMin = "0.00001", UIMin = "0.00001"))
	float DensityMultiplier = 1.0;

	// Describes the probability of scattering (versus absorption) at a scattering event. Between 0 and 1.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Principled Volume", meta = (UIMin = "0.0", UIMax = "0.95", ClampMin = "0.0", ClampMax = "0.95"))
	float Albedo = 0.8;

	// Backward or forward scattering direction (aka directional bias).
	// The default value of zero gives isotropic scattering so that light is scattered evenly in all directions. 
	// Positive values bias the scattering effect forwards, in the direction of the light, while negative values bias the scattering backward, toward the light. 
	// This shader uses the Henyey-Greenstein phase function.
	// Note that values very close to 1.0 (above 0.95) or -1.0 (below - 0.95) will produce scattering that is so directional that it will not be very visible from most angles, so such values are not recommended.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Principled Volume", meta = (ClampMin = "-1.0", UIMin = "-1.0", ClampMax = "1.0", UIMax = "1.0"))
	float Anisotropy = 0.0;

	// Add enum for emission mode 
	//	None: emit no light.
	//	Channel : emit light using a specified emission channel, or using a shader connected to the emission rate or color parameters.
	//	Density : emit light using the density channel, modulated by an optional emission channel.
	//	Blackbody : emit color and intensity based on temperature, for rendering fire and explosions.

	// Amount of light to emit.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Principled Volume", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float EmissionStrength = 0.0;

	// Emission color tint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Principled Volume")
	FLinearColor EmissionColor = FLinearColor(1.0, 1.0, 1.0, 1.0);

	// Blackbody emission for fire. Set to 1 for physically accurate intensity.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Principled Volume", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float BlackbodyIntensity = 1.0;

	// Color tint for blackbody emission.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Principled Volume")
	FLinearColor BlackbodyTint = FLinearColor(1.0, 1.0, 1.0, 1.0);

	// Temperature in kelvin for blackbody emission, higher values emit more.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Principled Volume", meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "6500", UIMax = "6500"))
	float Temperature = 1500.0;

	//----------------------------------------------------------------------------
	// Debug options (by order of priority)
	//----------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DebugDisplay)
	bool DisplayBounds = false;

	// Display samples in purple if they couldn't converge within RayDepth steps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DebugDisplay)
	bool DisplayUnfinishedPaths = false;

	//----------------------------------------------------------------------------

	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const  override;
	//~ End USceneComponent Interface.

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UVdbComponentBase Interface.
	virtual bool UpdateSceneProxy(uint32 FrameIndex, class UVdbVolumeSequence* VdbSequence) override;
#if WITH_EDITOR
	virtual void UpdateSeqProperties(const UVdbSequenceComponent* SeqComponent) override;
#endif
	//~ End UVdbComponentBase Interface.

	//const FVolumeRenderInfos* GetRenderInfos(UVdbVolumeBase* VdbVolume) const;

	EVdbType GetVdbType() const;
	void SetSeqComponents(UVdbSequenceComponent* CompDensity, UVdbSequenceComponent* CompTemperature) { SeqComponentDensity = CompDensity; SeqComponentTemperature = CompTemperature; }
	const UVdbSequenceComponent* GetSeqComponentDensity() const { return SeqComponentDensity; }
	const UVdbSequenceComponent* GetSeqComponentTemperature() const { return SeqComponentTemperature; }

protected:

	//~ Begin UObject Interface.
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface.

private:

	UVdbSequenceComponent* SeqComponentDensity;
	UVdbSequenceComponent* SeqComponentTemperature;
};

// VolumetricFog sparse data actor based on NanoVDB
// Experimental Actor, for research purpose only. Does not integrate into the rest of the pipeline well.
UCLASS(ClassGroup = Rendering, Meta = (ComponentWrapperClass))
class AVdbResearchActor : public AActor
{
	GENERATED_UCLASS_BODY()

private:

#if WITH_EDITOR
	virtual bool GetReferencedContentObjects(TArray<UObject*>& Objects) const override;
#endif

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Atmosphere, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UVdbResearchComponent> VdbComponent;

	UPROPERTY(BlueprintReadOnly, Category = Volumetric, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UVdbSequenceComponent> SeqDensComponent;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UVdbSequenceComponent> SeqTempComponent;
};