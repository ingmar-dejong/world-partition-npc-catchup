# NPC Schedule Catchup Behavior

This repository contains a focused extract of an Unreal Engine NPC schedule system that solves a specific World Partition problem:

When an NPC is unloaded while walking to a scheduled destination and later streams back in, the NPC should not restart from its original spawn point. Instead, it should resume from a believable midpoint on its route.

The code in this repo implements that catchup behavior with a small custom layer on top of an existing schedule / goal-based AI framework.

Link to showcase: https://onedrive.live.com/?id=%2Fpersonal%2F9d9f3cd318aeacbc%2FDocuments%2FPortfolio%2F4%2E%20NPC%20Schedule%20Catchup%20Behavior%20%28Prototype%29&viewid=b61252c1%2D4089%2D4b48%2Dba1f%2D36bf4fdf86eb

## Problem

In a schedule-driven AI setup, NPCs are often assigned goals based on time of day:

- leave home at `08:00`
- walk to market
- start an interaction at `09:00`

With World Partition enabled, the NPC may be unloaded while the player is far away. When the player returns, the NPC streams back in. At that moment, a naive schedule system typically does one of two wrong things:

- restart the walk from the original starting point
- place the NPC at the final interaction without any travel context

This system adds a middle ground:

- detect that the NPC is being restored during load / stream-in
- determine which route checkpoint should already have been reached
- reposition the NPC to that checkpoint
- let the normal AI flow continue from there

## What This Repo Contains

### Custom catchup layer

- [`AI/UnchainedEvent_AddGoalToNPC.h`](./AI/UnchainedEvent_AddGoalToNPC.h)
- [`AI/UnchainedEvent_AddGoalToNPC.cpp`](./AI/UnchainedEvent_AddGoalToNPC.cpp)
- [`AI/UnchainedNPCCatchupLibrary.h`](./AI/UnchainedNPCCatchupLibrary.h)
- [`AI/UnchainedNPCCatchupLibrary.cpp`](./AI/UnchainedNPCCatchupLibrary.cpp)
- [`AI/UnchainedNPCWalkRoute.h`](./AI/UnchainedNPCWalkRoute.h)
- [`AI/UnchainedNPCWalkRoute.cpp`](./AI/UnchainedNPCWalkRoute.cpp)

### Minimal dependency context

These files are included because the catchup logic depends on them conceptually:

- [`Dependencies/OwnSystemGoalItem.h`](./Dependencies/OwnSystemGoalItem.h)
- [`Dependencies/OwnSystemGoalItem.cpp`](./Dependencies/OwnSystemGoalItem.cpp)
- [`Dependencies/OwnSystemEvent_AddGoalToNPC.h`](./Dependencies/OwnSystemEvent_AddGoalToNPC.h)
- [`Dependencies/OwnSystemEvent_AddGoalToNPC.cpp`](./Dependencies/OwnSystemEvent_AddGoalToNPC.cpp)
- [`Dependencies/OwnSystemActivitySchedule.h`](./Dependencies/OwnSystemActivitySchedule.h)
- [`Dependencies/OwnSystemActivitySchedule.cpp`](./Dependencies/OwnSystemActivitySchedule.cpp)
- [`Dependencies/OwnSystemCharacter_CatchupExcerpt.h`](./Dependencies/OwnSystemCharacter_CatchupExcerpt.h)
- [`Dependencies/OwnSystemCharacter_CatchupExcerpt.cpp`](./Dependencies/OwnSystemCharacter_CatchupExcerpt.cpp)

`Dependencies/` is not intended as a full standalone framework. It is included to show the surrounding data flow that the custom catchup layer plugs into.

## Core Idea

The system separates catchup into two stages:

1. Goal setup stage
   When a scheduled goal is added during NPC load / stream-in, the system checks whether the NPC should already be partway along its route.

2. Runtime correction stage
   Just before the movement / interaction flow continues, the system applies a one-time teleport to the correct checkpoint and clears the catchup flag.

This keeps the custom logic narrow:

- the base schedule system still decides which goal is active
- the catchup layer only fixes physical world position for mid-walk recovery

## How It Works

### 1. Scheduled goals record intended start time

In [`Dependencies/OwnSystemActivitySchedule.cpp`](./Dependencies/OwnSystemActivitySchedule.cpp), scheduled goals store their intended start time:

- `OurGoal->IntendedTODStartTime = EventTime;`

That gives the broader AI framework enough context to know that a goal belongs to a schedule window rather than an ad hoc runtime trigger.

### 2. Goal state carries catchup flags

In [`Dependencies/OwnSystemGoalItem.h`](./Dependencies/OwnSystemGoalItem.h), the goal object contains the catchup state:

- `bHasCatchupCheckpoint`
- `CatchupCheckpointLocation`
- `IntendedTODStartTime`

This is the data contract between schedule creation, load detection, and the later movement correction.

### 3. Load-phase detection gates catchup

In [`AI/UnchainedEvent_AddGoalToNPC.cpp`](./AI/UnchainedEvent_AddGoalToNPC.cpp), the custom add-goal event checks:

- whether the NPC is still in its load / initialization phase
- whether the goal has a valid route time window

The relevant signal comes from [`Dependencies/OwnSystemCharacter_CatchupExcerpt.cpp`](./Dependencies/OwnSystemCharacter_CatchupExcerpt.cpp), where `IsCharacterPendingLoad()` represents "this NPC is not fully ready yet and is being restored during stream-in."

### 4. Route checkpoints are resolved from spline data

[`AI/UnchainedNPCWalkRoute.h`](./AI/UnchainedNPCWalkRoute.h) and [`AI/UnchainedNPCWalkRoute.cpp`](./AI/UnchainedNPCWalkRoute.cpp) define a level actor with a spline:

- spline point 0 = start
- last spline point = destination
- intermediate points = catchup checkpoints

Each intermediate point receives an automatically calculated expected time-of-day value based on distance along the spline.

### 5. The best checkpoint is selected from current world time

Still in [`AI/UnchainedEvent_AddGoalToNPC.cpp`](./AI/UnchainedEvent_AddGoalToNPC.cpp):

- the current world time is read from game state
- all checkpoints are compared against current time
- the furthest checkpoint that should already have been reached is selected

If none should have been reached yet, the system falls back to the first checkpoint so the NPC does not restart at the original route origin.

### 6. The correction is applied once

[`AI/UnchainedNPCCatchupLibrary.cpp`](./AI/UnchainedNPCCatchupLibrary.cpp) performs the actual recovery step:

- teleport the pawn to `CatchupCheckpointLocation`
- clear `bHasCatchupCheckpoint`
- reset the stored location
- stop current movement so the AI can resume cleanly

This makes the catchup idempotent and prevents repeated repositioning.

## Integration Pattern

This system is designed for projects that already have:

- schedule-driven NPC behavior
- a goal or task object per activity
- World Partition or another unload / reload flow
- a game-state time source

To integrate the approach into your own project:

1. Extend your base "Add Goal To NPC" event/class.
   Add a project-specific subclass similar to [`AI/UnchainedEvent_AddGoalToNPC.h`](./AI/UnchainedEvent_AddGoalToNPC.h).

2. Add route timing metadata.
   For each scheduled walk goal, define:
   - route start time
   - expected arrival time
   - either a spline route actor or manual checkpoints

3. Add catchup fields to your goal object.
   Your goal item needs at least:
   - a boolean "has catchup checkpoint"
   - a vector "checkpoint location"
   - optionally an intended schedule start time

4. Detect stream-in / load context.
   Your NPC class needs a reliable signal equivalent to `IsCharacterPendingLoad()` so the system can distinguish initial load catchup from normal runtime goal injection.

5. Apply the correction before normal movement resumes.
   The movement / interaction layer should call an equivalent of `TryApplyMidWalkCatchup(...)` once, then continue with the existing AI flow.

6. Author routes in the level.
   For each walk that should recover believably after stream-in:
   - place a route actor
   - draw the spline
   - ensure intermediate points represent meaningful travel positions

## Example Workflow

Example schedule:

- `08:00` NPC leaves house
- `09:00` NPC should reach workshop

Authoring:

- place a route spline from house to workshop
- add intermediate spline points at meaningful route milestones
- assign:
  - `StartTOD = 800`
  - `ExpectedArrivalTOD = 900`

Runtime scenario:

- player is away while the NPC is walking
- NPC unloads with World Partition
- player returns at `08:40`
- NPC streams back in

Catchup result:

- the system sees that the NPC is loading
- evaluates which checkpoint corresponds to `08:40`
- stores that checkpoint on the active goal
- teleports the NPC there once
- lets the normal AI continue toward the workshop

## Why This Approach Works Well

- It keeps custom logic small and local.
- It does not replace the base schedule system.
- It preserves believable spatial continuity.
- It avoids large save/load snapshots for every mid-route state.
- It scales well for authored schedule routes.

## Constraints and Assumptions

- This repo is a portfolio extract, not a drop-in plugin.
- Some included dependency files are partial context, not a complete framework.
- Names such as `OwnSystem` are anonymized replacements for the original project/plugin naming.
- The approach assumes a reliable world time source and route authoring discipline.
- The correction logic is aimed at mid-walk recovery, not every possible AI resimulation problem.

## Reuse Notes

If you want to reuse this pattern in another project, the most important files to study first are:

- [`AI/UnchainedEvent_AddGoalToNPC.cpp`](./AI/UnchainedEvent_AddGoalToNPC.cpp)
- [`AI/UnchainedNPCCatchupLibrary.cpp`](./AI/UnchainedNPCCatchupLibrary.cpp)
- [`AI/UnchainedNPCWalkRoute.cpp`](./AI/UnchainedNPCWalkRoute.cpp)
- [`Dependencies/OwnSystemGoalItem.h`](./Dependencies/OwnSystemGoalItem.h)

Those four pieces contain the essential design:

- detect catchup context
- convert a route into time-based checkpoints
- store the selected checkpoint on the active goal
- apply the correction once and resume normal AI behavior
