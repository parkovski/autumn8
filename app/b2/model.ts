export interface DirEntry {
  name: string;
  isDir: boolean;
};

export interface DirList {
  name: string;
  entries: DirEntry[];
};

export interface NavState {
  canGoBack: boolean;
  canGoForward: boolean;
  canGoUp: boolean;
};
