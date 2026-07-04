import { WorldTile } from '@/types/world';

export type AvatarOptionsJson = Record<string, unknown>;

export type Database = {
  public: {
    Tables: {
      avatars: {
        Row: {
          user_id: string;
          name: string | null;
          options: AvatarOptionsJson;
          created_at: string;
          updated_at: string;
          deleted_at: string | null;
        };
        Insert: {
          user_id: string;
          name?: string | null;
          options: AvatarOptionsJson;
          created_at?: string;
          updated_at?: string;
          deleted_at?: string | null;
        };
        Update: Partial<Database['public']['Tables']['avatars']['Insert']>;
        Relationships: [];
      };
      worlds: {
        Row: {
          user_id: string;
          name: string;
          tiles: WorldTile[];
          created_at: string;
          updated_at: string;
          deleted_at: string | null;
        };
        Insert: {
          user_id: string;
          name?: string;
          tiles: WorldTile[];
          created_at?: string;
          updated_at?: string;
          deleted_at?: string | null;
        };
        Update: Partial<Database['public']['Tables']['worlds']['Insert']>;
        Relationships: [];
      };
      quest_progress: {
        Row: {
          user_id: string;
          quest_key: string;
          progress: Record<string, unknown>;
          completed_at: string | null;
          created_at: string;
          updated_at: string;
          deleted_at: string | null;
        };
        Insert: {
          user_id: string;
          quest_key: string;
          progress?: Record<string, unknown>;
          completed_at?: string | null;
          created_at?: string;
          updated_at?: string;
          deleted_at?: string | null;
        };
        Update: Partial<Database['public']['Tables']['quest_progress']['Insert']>;
        Relationships: [];
      };
    };
    Views: Record<string, never>;
    Functions: Record<string, never>;
    Enums: Record<string, never>;
    CompositeTypes: Record<string, never>;
  };
};

export type WorldRow = Database['public']['Tables']['worlds']['Row'];
