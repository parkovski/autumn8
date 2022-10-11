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
};

export interface File extends DirEntry {
  kind: 'file';
  name: string;
  mimeType: string;
  data?: Buffer;
};

export interface Directory extends DirEntry {
  kind: 'directory';
  name: string;
  parent: Directory | null;
  entries: DirEntry[] | null;
};
