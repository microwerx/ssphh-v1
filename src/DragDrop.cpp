// For Drag and Drop
// This code incorporates ideas from http://www.catch22.net/tuts/win32/drop-source
// and from the Microsoft documentation website

#include <DragDrop.hpp>
#include <hatchetfish.hpp>

DragDrop dragDrop;

#ifdef _WIN32
#include <Windows.h>
#include <WinUser.h>
#include <shellapi.h> // for DragAcceptFiles()
#include <oleidl.h>		// for Drag and Drop
#include <shlobj_core.h> // for SH...

//////////////////////////////////////////////////////////////////////
// H E L P E R   F U N C T I O N S ///////////////////////////////////
//////////////////////////////////////////////////////////////////////

HGLOBAL DupGlobalMem(HGLOBAL hMem);
HRESULT CreateDataObject(IDataObject **ppDataObject, FORMATETC *fmtetc, STGMEDIUM *stgmeds, UINT count);
HANDLE StringToHandle(char *szText);

//////////////////////////////////////////////////////////////////////
// M y D a t a O b j e c t ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


class MyDataObject : public IDataObject
{
private:
	LONG referenceCount_;
	LONG numFormats_;
	std::vector<FORMATETC> pFormatEtcs_;
	std::vector<STGMEDIUM> pStgMediums_;

	int LookupFormatEtc(FORMATETC *pFormatEtc)
	{
		// check each of our formats in turn to see if one matches
		for (int i = 0; i < numFormats_; i++) {
			if ((pFormatEtcs_[i].tymed & pFormatEtc->tymed) &&
				pFormatEtcs_[i].cfFormat == pFormatEtc->cfFormat &&
				pFormatEtcs_[i].dwAspect == pFormatEtc->dwAspect) {
				// return index of stored format
				return i;
			}
		}

		// error, format not found
		return -1;
	}
public:
	MyDataObject(FORMATETC *pformatetc, STGMEDIUM *pmedium, int count)
	{
		referenceCount_ = 1;
		numFormats_ = count;

		pFormatEtcs_.resize(count);
		pStgMediums_.resize(count);

		for (int i = 0; i < count; i++) {
			pFormatEtcs_[i] = pformatetc[i];
			pStgMediums_[i] = pmedium[i];
		}
	}

	~MyDataObject()
	{

	}

	// IUnknown members
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) override
	{
		// tell other objects about our capabilities 
		if (riid == IID_IUnknown || riid == IID_IDropTarget) {
			*ppvObject = this;
			AddRef();
			return NOERROR;
		}
		*ppvObject = NULL;
		return ResultFromScode(E_NOINTERFACE);
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void) override
	{
		return referenceCount_++;
	}

	virtual ULONG STDMETHODCALLTYPE Release(void) override
	{
		if (--referenceCount_ == 0) {
			delete this;
			return 0;
		}
		return referenceCount_;
	}

	// IDataObject members
	HRESULT STDMETHODCALLTYPE GetData(FORMATETC *pFormatEtc, STGMEDIUM *pStgMedium) override
	{
		int idx;

		// try to match the specified FORMATETC with one of our supported formats
		if ((idx = LookupFormatEtc(pFormatEtc)) == -1)
			return DV_E_FORMATETC;

		// found a match - transfer data into supplied storage medium
		pStgMedium->tymed = pFormatEtcs_[idx].tymed;
		pStgMedium->pUnkForRelease = 0;

		// copy the data into the caller's storage medium
		switch (pFormatEtcs_[idx].tymed) {
		case TYMED_HGLOBAL:
			pStgMedium->hGlobal = DupGlobalMem(pStgMediums_[idx].hGlobal);
			break;

		default:
			return DV_E_FORMATETC;
		}
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC *pFormatEtc, STGMEDIUM *pmedium) override
	{
		return DATA_E_FORMATETC;
	}
	HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC *pFormatEtc) override
	{
		return (LookupFormatEtc(pFormatEtc) == -1) ? DV_E_FORMATETC : S_OK;
	}
	HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut) override
	{
		return DATA_S_SAMEFORMATETC;
	}
	HRESULT STDMETHODCALLTYPE SetData(FORMATETC *pFormatEtc, STGMEDIUM *pMedium, BOOL fRelease) override
	{
		return E_NOTIMPL;
	}
	HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc) override
	{
		if (dwDirection != DATADIR_GET) {
			return E_NOTIMPL;
		}
		return SHCreateStdEnumFmtEtc((int)pFormatEtcs_.size(), pFormatEtcs_.data(), ppEnumFormatEtc);
	}
	HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *, DWORD *) override
	{
		return E_NOTIMPL;
	}
	HRESULT STDMETHODCALLTYPE DUnadvise(DWORD dwConnection) override
	{
		return E_NOTIMPL;
	}
	HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA **ppEnumAdvise) override
	{
		return E_NOTIMPL;
	}
};

class MyDropTarget : public IDropTarget
{
private:
	ULONG referenceCount_;
	bool acceptFormat;
	bool mergeIncoming;
	bool replaceIncoming;
	HWND window;
	DWORD dropEffect;
	DragDrop *dragDrop_ = nullptr;

	bool QueryDataObject(IDataObject *pDataObject)
	{
		acceptFormat = false;
		if (!pDataObject) return false;

		IEnumFORMATETC *ef;
		FORMATETC formats[20];
		ULONG fetched = 0;

		// Enumerate available items:
		if (pDataObject->EnumFormatEtc(DATADIR_GET, &ef) == S_OK) {
			ef->Next(10, formats, &fetched);
			for (unsigned i = 0; i < fetched; i++) {
				if (formats[i].cfFormat < CF_MAX) {
					HFLOGINFO("CF[%i] (cfFormat, Aspect, Index, Tymed) => (%i, %i, %i %i)",
						i,
						formats[i].cfFormat,
						formats[i].dwAspect,
						formats[i].lindex,
						formats[i].tymed);
				}
				else {
					HFLOGINFO("CF[%i] (cfFormat, ...) => (%i, ...)", i, formats[i].cfFormat);
				}
			}
			ef->Release();
		}

		FORMATETC fmtetc[] = {
			{ CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
			{ CF_TEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
			{ CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
			{ 0, nullptr, 0, 0, 0 }
		};
		FORMATETC *fmt = fmtetc;
		while (fmt->cfFormat > 0) {
			if (pDataObject->QueryGetData(fmt) == S_OK) {
				acceptFormat = true;
			}
			fmt++;
		}
		return acceptFormat;
	}

	HRESULT DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed)
	{
		replaceIncoming = true;
		// Maybe we want to merge or replace scene file?
		if (grfKeyState & MK_CONTROL) {
			replaceIncoming = false;
		}
		if (grfKeyState & MK_SHIFT) {
			replaceIncoming = true;
		}
		mergeIncoming = !replaceIncoming;
		return dwAllowed & DROPEFFECT_COPY;
	}

	void CopyPaths(FORMATETC *fmt, STGMEDIUM *medium)
	{
		std::vector<std::string> &paths_ = dragDrop_->paths_;
		if (!fmt || fmt->cfFormat != CF_HDROP) return;
		SIZE_T bytes = GlobalSize(medium->hGlobal);
		HDROP hDrop = (HDROP)GlobalLock(medium->hGlobal);
		UINT count = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		if (count > 0) {
			paths_.resize(count);
			for (UINT i = 0; i < count; i++) {
				UINT bufferSize = DragQueryFile(hDrop, i, NULL, 0);
				paths_[i].resize(bufferSize + 1);
				DragQueryFile(hDrop, i, (char *)paths_[i].data(), bufferSize + 1);
			}
		}
		GlobalUnlock(medium->hGlobal);
		if (!paths_.empty()) {
			for (auto & path : paths_) {
				HFLOGINFO("%s", path.c_str());
			}
			dragDrop_->gotPaths_ = true;
		}
		else {
			dragDrop_->gotPaths_ = false;
		}
	}

	void CopyText(FORMATETC *fmt, STGMEDIUM *medium)
	{
		std::string &text_ = dragDrop_->text_;

		if (!fmt || fmt->cfFormat != CF_TEXT) return;
		SIZE_T bytes = GlobalSize(medium->hGlobal);
		char *buffer = (char *)GlobalLock(medium->hGlobal);
		text_.resize(bytes + 1);
		memcpy((LPVOID)text_.data(), buffer, bytes);
		GlobalUnlock(medium->hGlobal);
		if (bytes > 1) {
			dragDrop_->gotText_ = true;
			HFLOGINFO("%s", text_.c_str());
		}
		else {
			dragDrop_->gotText_ = false;
		}
	}

	void CopyUnicode(FORMATETC *fmt, STGMEDIUM *medium)
	{
		std::wstring &unicodeText_ = dragDrop_->unicodeText_;

		if (!fmt || fmt->cfFormat != CF_UNICODETEXT) return;
		SIZE_T bytes = GlobalSize(medium->hGlobal);
		char *buffer = (char *)GlobalLock(medium->hGlobal);
		unicodeText_.resize(bytes + 1);
		memcpy((LPVOID)unicodeText_.data(), buffer, bytes);
		GlobalUnlock(medium->hGlobal);
		if (bytes > 1) {
			dragDrop_->gotUnicodeText_ = true;
			HFLOGINFO("%S", unicodeText_.c_str());
		}
		else {
			dragDrop_->gotUnicodeText_ = false;
		}
	}

	HRESULT ProcessDropData(IDataObject *pDataObject)
	{
		if (!pDataObject) return E_INVALIDARG;
		FORMATETC fmtetc[] = {
			{ CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
			{ CF_TEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
			{ CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
			{ 0, nullptr, 0, 0, 0 }
		};
		FORMATETC *fmt = fmtetc;
		while (fmt->cfFormat > 0) {
			if (pDataObject->QueryGetData(fmt) == S_OK) {
				STGMEDIUM medium;
				if (pDataObject->GetData(fmt, &medium) == S_OK) {
					if (medium.tymed == fmt->tymed) {
						if (fmt->cfFormat == CF_HDROP) { CopyPaths(fmt, &medium); }
						else if (fmt->cfFormat == CF_TEXT) { CopyText(fmt, &medium); }
						else if (fmt->cfFormat == CF_UNICODETEXT) { CopyUnicode(fmt, &medium); }
					}
					ReleaseStgMedium(&medium);
				}
			}
			fmt++;
		}
		HFLOGINFO("%s", replaceIncoming ? "Replacing" : "Merging");
		return S_OK;
	}
public:
	MyDropTarget(DragDrop *pDragDrop)
	{
		window = GetActiveWindow();
		acceptFormat = false;
		referenceCount_ = 1;
		dropEffect = 0;
		dragDrop_ = pDragDrop;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) override
	{
		// tell other objects about our capabilities 
		if (riid == IID_IUnknown || riid == IID_IDropTarget) {
			*ppvObject = this;
			AddRef();
			return NOERROR;
		}
		*ppvObject = NULL;
		return ResultFromScode(E_NOINTERFACE);
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void) override
	{
		return referenceCount_++;
	}

	virtual ULONG STDMETHODCALLTYPE Release(void) override
	{
		if (--referenceCount_ == 0) {
			delete this;
			return 0;
		}
		return referenceCount_;
	}

	virtual HRESULT STDMETHODCALLTYPE DragEnter(
		/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect) override
	{
		HFLOGINFO("DragEnter");

		*pdwEffect = DropEffect(grfKeyState, pt, DROPEFFECT_COPY);

		if (QueryDataObject(pDataObj)) {
			dragDrop_->Reset();
			SetFocus(window);
		}
		else {
			*pdwEffect = DROPEFFECT_NONE;
		}

		//// This old code is if we are acting as a source, so save for a later example
		//// Create acceptable formats
		//FORMATETC fmtetc = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		//STGMEDIUM stgmed = { TYMED_HGLOBAL, { 0 }, 0 };

		//stgmed.hGlobal = StringToHandle("Hello, World!");

		//// Create drag source
		//MyDropSource *pDropSource = new MyDropSource();
		//MyDataObject *pDataObject = nullptr;
		//pDropSource->AddRef();
		//pDataObject->AddRef();
		//CreateDataObject(&pDataObject, &fmtetc, &stgmed, 1);

		//DWORD result = DoDragDrop(pDataObject, pDropSource, DROPEFFECT_COPY, &dropEffect);
		//if (result == DRAGDROP_S_DROP && (dropEffect & DROPEFFECT_MOVE)) {
		//	// remove data dropped from this object
		//}

		//pDropSource->Release();
		//pDataObject->Release();
		//ReleaseStgMedium(&stgmed);
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DragOver(
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect) override
	{
		if (acceptFormat) {
			DropEffect(grfKeyState, pt, *pdwEffect);
			*pdwEffect = replaceIncoming ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
		}
		else {
			*pdwEffect = DROPEFFECT_NONE;
		}
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DragLeave(void) override
	{
		//Hf::Log.infofn(__FUNCTION__, "DragLeave");
		HFLOGINFO("DragLeave");
		return NOERROR;
	}

	virtual HRESULT STDMETHODCALLTYPE Drop(
		/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect) override
	{
		if (acceptFormat) {
			*pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
			ProcessDropData(pDataObj);
		}
		else {
			*pdwEffect = DROPEFFECT_NONE;
		}

		// Do something here
		FORMATETC format;
		STGMEDIUM storageMedium;
		if (pDataObj->GetData(&format, &storageMedium) == S_OK) {
			HFLOGINFO("Drop (%d, %d, %d, %d)", format.cfFormat, format.dwAspect, format.lindex, format.tymed);
		}
		return S_OK;
	}
};


class MyDropSource : public IDropSource
{
private:
	ULONG referenceCount_;
	bool acceptFormat;
	HWND window;

public:
	MyDropSource()
	{
		window = GetActiveWindow();
		acceptFormat = false;
		referenceCount_ = 1;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) override
	{
		// tell other objects about our capabilities 
		if (riid == IID_IUnknown || riid == IID_IDropTarget) {
			*ppvObject = this;
			AddRef();
			return NOERROR;
		}
		*ppvObject = NULL;
		return ResultFromScode(E_NOINTERFACE);
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void) override
	{
		return referenceCount_++;
	}

	virtual ULONG STDMETHODCALLTYPE Release(void) override
	{
		if (--referenceCount_ == 0) {
			delete this;
			return 0;
		}
		return referenceCount_;
	}

	virtual HRESULT STDMETHODCALLTYPE GiveFeedback(
		/* [in] */ DWORD dwEffect
	) override
	{
		return DRAGDROP_S_USEDEFAULTCURSORS;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryContinueDrag(
		/* [in] */ BOOL fEscapePressed,
		/* [in] */ DWORD grfKeyState
	) override
	{
		// ESCAPE key or right mouse button pressed causes cancel
		if (fEscapePressed || (grfKeyState & MK_RBUTTON)) {
			return DRAGDROP_S_CANCEL;
			HFLOGINFO("Cancelling drop");
		}

		// Left mouse button released causes drop
		if ((grfKeyState & MK_LBUTTON) == 0) {
			HFLOGINFO("Dropping");
			return DRAGDROP_S_DROP;
		}

		return S_OK;
	}
};

//////////////////////////////////////////////////////////////////////
// H E L P E R   F U N C T I O N S ///////////////////////////////////
//////////////////////////////////////////////////////////////////////

HGLOBAL DupGlobalMem(HGLOBAL hMem)
{
	SIZE_T len = GlobalSize(hMem);
	PVOID source = GlobalLock(hMem);
	PVOID dest = GlobalAlloc(GMEM_FIXED, len);
	memcpy(dest, source, len);
	GlobalUnlock(hMem);
	return dest;
}


HANDLE StringToHandle(char *szText)
{
	char *buffer = nullptr;
	size_t bufferSize = lstrlen(szText) + 1;
	buffer = (char *)GlobalAlloc(GMEM_FIXED, bufferSize);
	memcpy(buffer, szText, bufferSize - 1);
	buffer[bufferSize] = '\0';
	return buffer;
}

HRESULT CreateDataObject(IDataObject **ppDataObject, FORMATETC *fmtetc, STGMEDIUM *stgmeds, UINT count)
{
	if (ppDataObject == 0)
		return E_INVALIDARG;

	*ppDataObject = new MyDataObject(fmtetc, stgmeds, count);

	return (*ppDataObject) ? S_OK : E_OUTOFMEMORY;
}
#endif


DragDrop::DragDrop()
{
}


DragDrop::~DragDrop()
{
	if (started) {
		Kill();
	}
}


void DragDrop::Init()
{
	started = true;
#ifdef _WIN32
	Hf::Log.infofn(__FUNCTION__, "Starting Drag and Drop");
	HRESULT oleInitialized = OleInitialize(nullptr);
	HWND hwnd = GetActiveWindow();
	if (!hwnd) return;
	DragAcceptFiles(hwnd, TRUE);
	pDropTarget = new MyDropTarget(this);
	HRESULT result = RegisterDragDrop(hwnd, (IDropTarget *)pDropTarget);
#endif
}


void DragDrop::Kill()
{
	started = false;
#ifdef _WIN32
	if (pDropTarget) {
		if (pDropTarget->Release() == 0) {
			pDropTarget = nullptr;
		};
	}
	OleUninitialize();
#endif
}


void DragDrop::Reset()
{
	gotText_ = false;
	gotUnicodeText_ = false;
	gotPaths_ = false;
	text_.resize(0);
	unicodeText_.resize(0);
	paths_.resize(0);
}
