-- A simple synthesizer voice-assignment implementation in Lua
-- converted from voice_tracker.c
-- (c) seclorun 2025
-- MIT Licensed - see LICENSE

local MAX_VOICES = 8
local NOTE_ON = 1
local NOTE_OFF = 0

-- to get access to higher resolution, ffi might be used:
local ffi = require("ffi")
ffi.cdef[[
    typedef long time_t;
    typedef struct timespec { time_t tv_sec; long tv_nsec; } timespec;
    int clock_gettime(int clk_id, struct timespec *tp);
]]

local function current_time_ns()
    local ts = ffi.new("struct timespec")
    ffi.C.clock_gettime(1, ts) -- CLOCK_MONOTONIC
    return ts.tv_sec * 1000000000 + ts.tv_nsec
end

-- Timestamp in nanoseconds
local function current_time_ms()
    return os.time() * 1000000000 -- Approximate replacement
end

-- Deque implementation
local Deque = {}
Deque.__index = Deque

function Deque:new()
    return setmetatable({ front = {}, rear = {}, size = 0 }, self)
end

function Deque:isEmpty()
    return self.size == 0
end

function Deque:noteExists(note)
    for _, v in ipairs(self.front) do
        if v == note then return true end
    end
    return false
end

function Deque:pushFront(note)
    if self:noteExists(note) then return end
    table.insert(self.front, 1, note)
    self.size = self.size + 1
end

function Deque:pushBack(note)
    if self:noteExists(note) then return end
    table.insert(self.front, note)
    self.size = self.size + 1
end

function Deque:popBack()
    if self:isEmpty() then return -1 end
    local note = table.remove(self.front)
    self.size = self.size - 1
    return note
end

function Deque:removeNote(note)
    for i, v in ipairs(self.front) do
        if v == note then
            table.remove(self.front, i)
            self.size = self.size - 1
            return
        end
    end
end

function Deque:printContents()
    io.write("Note state as of [" .. tostring(current_time_ms()) .. "]: ")
    if self:isEmpty() then
        print("None")
    else
        print(table.concat(self.front, ", "))
    end
end

local function synth_voice_ts(voicenum, state, dq)
    print(string.format("Voice %d: [%d ns] %s", voicenum, current_time_ms(), state == NOTE_ON and "NOTE ON" or "NOTE OFF"))
    dq:printContents()
end

local function noteOn(dq, note)
    if dq.size >= MAX_VOICES then
        local stolenNote = dq:popBack()
        if stolenNote ~= -1 then
            synth_voice_ts(stolenNote, NOTE_OFF, dq)
        end
    end
    dq:pushFront(note)
    synth_voice_ts(note, NOTE_ON, dq)
end

local function noteOff(dq, note)
    dq:removeNote(note)
    synth_voice_ts(note, NOTE_OFF, dq)
end

local function random_note_event(dq)
    local note = math.random(60, 72)
    local state = math.random(0, 1)
    
    if state == NOTE_ON then
        noteOn(dq, note)
    else
        noteOff(dq, note)
    end
end

-- Main execution
local dq = Deque:new()
noteOn(dq, 60)
noteOn(dq, 62)
noteOn(dq, 64)
noteOn(dq, 65)
noteOn(dq, 67)
noteOn(dq, 69)
noteOn(dq, 71)
noteOn(dq, 72)
noteOn(dq, 74)
noteOff(dq, 69)
noteOff(dq, 64)
noteOff(dq, 65)
noteOn(dq, 69)
noteOn(dq, 64)
noteOn(dq, 39)
noteOn(dq, 49)
noteOn(dq, 89)
noteOff(dq, 69)
noteOn(dq, 99)
noteOff(dq, 49)

for _ = 1, 20 do
    random_note_event(dq)
end

