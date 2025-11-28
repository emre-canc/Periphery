// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionData.h"

FPrimaryAssetId UMissionData::GetPrimaryAssetId() const
{
    // If MissionID is empty, it falls back to the file name.
    if (MissionID.IsValid())
    {
        return FPrimaryAssetId(GetClass()->GetFName(), MissionID.GetTagName());
    }

    return FPrimaryAssetId(GetClass()->GetFName(), GetFName());
}

#if WITH_EDITOR
void UMissionData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // If we changed the "NextMission" dropdown...
    if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UMissionData, NextMissionAsset))
    {
        // ... and it's not empty
        if (!NextMissionAsset.IsNull())
        {
            // Load it briefly to grab its ID
            if (UMissionData* NextAsset = NextMissionAsset.LoadSynchronous())
            {
                NextMissionID = NextAsset->MissionID;
            }
        }
        else
        {
            NextMissionID = FGameplayTag();
        }
    }
}
#endif