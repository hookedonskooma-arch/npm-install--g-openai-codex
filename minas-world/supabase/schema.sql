-- Mina's World — Handoff 11 schema (avatar, world map, quest progress)
--
-- Assumes Stiki SSO is wired into Supabase as a third-party JWT issuer, so
-- auth.uid() resolves to the `sub` claim of the RS256 JWT. Every table is
-- owned by a single auth.users(id) row, soft-deleted (never hard-deleted),
-- and locked down with RLS so a user can only ever touch their own rows.
--
-- Safe to re-run: every object creation is guarded.

create extension if not exists pgcrypto;

-- ---------------------------------------------------------------------------
-- updated_at trigger helper
-- ---------------------------------------------------------------------------
create or replace function public.set_updated_at()
returns trigger
language plpgsql
as $$
begin
  new.updated_at = now();
  return new;
end;
$$;

-- ---------------------------------------------------------------------------
-- avatars
-- ---------------------------------------------------------------------------
create table if not exists public.avatars (
  id uuid primary key default gen_random_uuid(),
  user_id uuid not null references auth.users (id),
  name text,
  options jsonb not null,
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now(),
  deleted_at timestamptz
);

create index if not exists avatars_user_id_idx on public.avatars (user_id);

-- one active (non-deleted) avatar per user
create unique index if not exists avatars_one_active_per_user
  on public.avatars (user_id)
  where deleted_at is null;

drop trigger if exists set_avatars_updated_at on public.avatars;
create trigger set_avatars_updated_at
  before update on public.avatars
  for each row execute function public.set_updated_at();

alter table public.avatars enable row level security;

drop policy if exists "avatars_select_own" on public.avatars;
create policy "avatars_select_own" on public.avatars
  for select using (auth.uid() = user_id);

drop policy if exists "avatars_insert_own" on public.avatars;
create policy "avatars_insert_own" on public.avatars
  for insert with check (auth.uid() = user_id);

drop policy if exists "avatars_update_own" on public.avatars;
create policy "avatars_update_own" on public.avatars
  for update using (auth.uid() = user_id) with check (auth.uid() = user_id);

-- No delete policy: rows are retired via soft delete (UPDATE deleted_at),
-- never removed with DELETE.

-- ---------------------------------------------------------------------------
-- worlds
-- ---------------------------------------------------------------------------
create table if not exists public.worlds (
  id uuid primary key default gen_random_uuid(),
  user_id uuid not null references auth.users (id),
  name text not null default 'My World',
  tiles jsonb not null,
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now(),
  deleted_at timestamptz
);

create index if not exists worlds_user_id_idx on public.worlds (user_id);

-- one active (non-deleted) world per user, matching the single local-save slot
create unique index if not exists worlds_one_active_per_user
  on public.worlds (user_id)
  where deleted_at is null;

drop trigger if exists set_worlds_updated_at on public.worlds;
create trigger set_worlds_updated_at
  before update on public.worlds
  for each row execute function public.set_updated_at();

alter table public.worlds enable row level security;

drop policy if exists "worlds_select_own" on public.worlds;
create policy "worlds_select_own" on public.worlds
  for select using (auth.uid() = user_id);

drop policy if exists "worlds_insert_own" on public.worlds;
create policy "worlds_insert_own" on public.worlds
  for insert with check (auth.uid() = user_id);

drop policy if exists "worlds_update_own" on public.worlds;
create policy "worlds_update_own" on public.worlds
  for update using (auth.uid() = user_id) with check (auth.uid() = user_id);

-- ---------------------------------------------------------------------------
-- quest_progress
-- ---------------------------------------------------------------------------
create table if not exists public.quest_progress (
  id uuid primary key default gen_random_uuid(),
  user_id uuid not null references auth.users (id),
  quest_key text not null,
  progress jsonb not null default '{}'::jsonb,
  completed_at timestamptz,
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now(),
  deleted_at timestamptz
);

create index if not exists quest_progress_user_id_idx on public.quest_progress (user_id);

-- one active row per user per quest
create unique index if not exists quest_progress_one_active_per_user_quest
  on public.quest_progress (user_id, quest_key)
  where deleted_at is null;

drop trigger if exists set_quest_progress_updated_at on public.quest_progress;
create trigger set_quest_progress_updated_at
  before update on public.quest_progress
  for each row execute function public.set_updated_at();

alter table public.quest_progress enable row level security;

drop policy if exists "quest_progress_select_own" on public.quest_progress;
create policy "quest_progress_select_own" on public.quest_progress
  for select using (auth.uid() = user_id);

drop policy if exists "quest_progress_insert_own" on public.quest_progress;
create policy "quest_progress_insert_own" on public.quest_progress
  for insert with check (auth.uid() = user_id);

drop policy if exists "quest_progress_update_own" on public.quest_progress;
create policy "quest_progress_update_own" on public.quest_progress
  for update using (auth.uid() = user_id) with check (auth.uid() = user_id);
