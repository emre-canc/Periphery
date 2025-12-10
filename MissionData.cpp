// Fill out your copyright notice in the Description page of Project Settings.


#include "Missions/MissionData.h"

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
    // 2. Handle Auto-Start for First Objective (New Logic)
    // We check if the "ObjectiveArray" was touched
    if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UMissionData, ObjectiveArray) 
        || PropertyChangedEvent.GetPropertyName() == NAME_None) // NAME_None catches some array reordering events
    {
        UMissionObjective* Obj;
        for (int32 i = 0; i < ObjectiveArray.Num(); i++)
        {
            Obj = ObjectiveArray[i];
            if (Obj)
            {
                // If it's Index 0, set true. Otherwise, set false.
                bool bShouldAutoStart = (i == 0);

                if (Obj->bStartAutomatically != bShouldAutoStart)
                {
                    Obj->Modify(); // Mark as dirty for saving
                    Obj->bStartAutomatically = bShouldAutoStart;
                }
            }
        }
    }
}
#endif