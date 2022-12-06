export type FileKind =
  | 'file'
  | 'directory'
  | 'symlink'
  | 'fifo'
  | 'socket'
  | 'reparse'
  | null;

export interface DirEntry {
  kind: FileKind;
  name: string;
  parent?: Directory | null;
};

export interface File extends DirEntry {
  kind: 'file';
  mimeType: string;
  data?: Buffer;
};

export interface Directory extends DirEntry {
  kind: 'directory';
  entries?: DirEntry[];
};

export interface Symlink extends DirEntry {
  kind: 'symlink';
  target: string;
};

export interface Reparse extends DirEntry {
  kind: 'reparse';
  tag: string;
  wellKnownTag?: string;
};
