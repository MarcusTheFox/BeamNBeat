#include "Core/Parsing/BeatMapParser.h"

bool FBeatMapParser::Parse(const TArray<FString>& Lines, TArray<FBeatMapData>& OutBeatMap, FBeatMapSettings& OutSettings)
{
	OutBeatMap.Empty();

	int32 CurrentLineIndex = 0;

	ParseHeader(Lines, CurrentLineIndex, OutSettings);
	ParseNotes(Lines, CurrentLineIndex, OutSettings, OutBeatMap);
	
	return true;
}

int32 FBeatMapParser::CountTargets(const TArray<FString>& Lines)
{
	int32 TargetCount = 0;
	int32 StartLineIndex = 0;
	
	if (Lines.Num() <= 0 || Lines[0].TrimStartAndEnd() != TEXT("---")) return 0;
	
	for (int32 i = 1; i < Lines.Num(); ++i)
	{
		if (Lines[i].TrimStartAndEnd() == TEXT("---"))
		{
			StartLineIndex = i + 1;
			break;
		}
	}

	for (int32 i = StartLineIndex; i < Lines.Num(); ++i)
	{
		const FString Line = Lines[i].TrimStartAndEnd();
		if (LineIsComment(Line)) continue;

		FString TimestampStr, EventsStr;
		if (!Line.Split(":", &TimestampStr, &EventsStr)) continue;

		TArray<FString> EventItems;
		EventsStr.ParseIntoArray(EventItems, TEXT(";"));

		for (const FString& EventItem : EventItems)
		{
			const FString CleanEventItem = EventItem.TrimStartAndEnd();
			
			if (CleanEventItem.StartsWith("@")) continue;

			TArray<FString> NoteDefinitions;
			CleanEventItem.ParseIntoArray(NoteDefinitions, TEXT(","));

			for (const FString& NoteDef : NoteDefinitions)
			{
				const FString TrimmedDef = NoteDef.TrimStartAndEnd();
				
				if (!TrimmedDef.IsEmpty())
				{
					TargetCount++;
				}
			}
		}
	}
	
	return TargetCount;
}

void FBeatMapParser::ParseHeader(const TArray<FString>& Lines, int32& InOutLineIndex, FBeatMapSettings& OutSettings)
{
	if (Lines.Num() == 0 || Lines[0].TrimStartAndEnd() != TEXT("---")) return;

	InOutLineIndex = 1;
	for (; InOutLineIndex < Lines.Num(); InOutLineIndex++)
	{
		const FString& Line = Lines[InOutLineIndex].TrimStartAndEnd();

		if (Line == TEXT("---"))
		{
			InOutLineIndex++;
			break;
		}

		if (!LineIsComment(Line))
		{
			FString Key, Value;
			if (Line.Split(":", &Key, &Value))
			{
				Key = Key.TrimStartAndEnd();
				Value = Value.TrimStartAndEnd();

				if (Key.Equals("bpm", ESearchCase::IgnoreCase))
					OutSettings.StartBPM = FCString::Atof(*Value);
				else if (Key.Equals("power", ESearchCase::IgnoreCase))
					OutSettings.DefaultPower = FCString::Atoi(*Value);
				else if (Key.Equals(TEXT("offset"), ESearchCase::IgnoreCase))
					OutSettings.TimeOffset = FCString::Atof(*Value); 
			}
		}
	}
}

void FBeatMapParser::ParseNotes(const TArray<FString>& Lines, int32 StartLineIndex, const FBeatMapSettings& Settings,
	TArray<FBeatMapData>& OutBeatMap)
{
	float CurrentBPM = Settings.StartBPM;
	int32 CurrentDefaultPower = Settings.DefaultPower;
	// TODO: Добавить переменные для CurrentDefaultColor и т.д.
	
	if (OutBeatMap.IsEmpty())
	{
		FBeatMapData InitialBpmMarker;
		InitialBpmMarker.SpawnerID = -1;
		InitialBpmMarker.ShotPower = 0;
		InitialBpmMarker.Beat = 0;
		InitialBpmMarker.BPM = CurrentBPM;
		OutBeatMap.Add(InitialBpmMarker);
	}

	for (int32 i = StartLineIndex; i < Lines.Num(); i++)
	{
		const FString Line = Lines[i].TrimStartAndEnd();
		if (LineIsComment(Line)) continue;

		FString TimestampStr, EventStr;
		if (!Line.Split(":", &TimestampStr, &EventStr)) continue;

		const float TotalBeat = FCString::Atof(*TimestampStr);

		TArray<FString> EventItems;
		EventStr.TrimStartAndEnd().ParseIntoArray(EventItems, TEXT(";"));

		float BpmForNextNote = 0.0f;
		
		for (const FString& EventItem : EventItems)
		{
			FString CleanItem = EventItem.TrimStartAndEnd();
			if (CleanItem.StartsWith("@"))
			{
				FString Command, Value;
				if (CleanItem.Mid(1).Split(":", &Command, &Value))
				{
					Command = Command.TrimStartAndEnd();
					Value = Value.TrimStartAndEnd();

					if (Command.Equals("BPM", ESearchCase::IgnoreCase))
					{
						BpmForNextNote = FCString::Atof(*Value);
					}
					else if (Command.Equals("Power", ESearchCase::IgnoreCase))
					{
						CurrentDefaultPower = FCString::Atoi(*Value);
					}
					// TODO: Добавить обработку других команд @
				}
			}
			else
			{
				FBeatMapSettings CurrentEventSettings = Settings;
				CurrentEventSettings.DefaultPower = CurrentDefaultPower;
				
				ParseNoteEvent(CleanItem, TotalBeat, BpmForNextNote, CurrentEventSettings, OutBeatMap);
				BpmForNextNote = 0.0f;
			}
		}

		if (BpmForNextNote > 0.0f)
		{
			FBeatMapData BpmMarker;
			BpmMarker.SpawnerID = -1;
			BpmMarker.ShotPower = 0;
			BpmMarker.Beat = TotalBeat;
			BpmMarker.BPM = BpmForNextNote;
			OutBeatMap.Add(BpmMarker);
		}
	}
}

void FBeatMapParser::ParseNoteEvent(const FString& NoteEventString, float TotalBeat, float BpmForThisTimestamp,
	const FBeatMapSettings& Settings, TArray<FBeatMapData>& OutBeatMap)
{
	TArray<FString> NoteItems;
	NoteEventString.ParseIntoArray(NoteItems, TEXT(","));

	bool bBpmApplied = false;

	for (const FString& NoteItem : NoteItems)
	{
		FString CleanNoteItem = NoteItem.TrimStartAndEnd();
		if (CleanNoteItem.IsEmpty()) continue;

		FBeatMapData NewData;
		NewData.ShotPower = Settings.DefaultPower;
		NewData.Beat = TotalBeat;
		NewData.BPM = 0.0f;

		if (BpmForThisTimestamp > 0.0f && !bBpmApplied)
		{
			NewData.BPM = BpmForThisTimestamp;
			bBpmApplied = true;
		}

		FString IdString = CleanNoteItem;
		const int32 PowerStartPos = CleanNoteItem.Find(TEXT("("));
		const int32 PowerEndPos = CleanNoteItem.Find(TEXT(")"));

		if (PowerStartPos != INDEX_NONE && PowerEndPos != INDEX_NONE && PowerEndPos > PowerStartPos)
		{
			IdString = CleanNoteItem.Left(PowerStartPos);
			const FString PowerString = CleanNoteItem.Mid(PowerStartPos + 1, PowerEndPos - PowerStartPos - 1);
			NewData.ShotPower = FCString::Atoi(*PowerString);
		}
		
		// TODO: Здесь будет парсинг { ... }

		NewData.SpawnerID = FCString::Atoi(*IdString.TrimStartAndEnd());

		OutBeatMap.Add(NewData);
	}
}

bool FBeatMapParser::LineIsComment(const FString& Line)
{
	return Line.IsEmpty() || Line.StartsWith("#") || Line.StartsWith("//");
}
