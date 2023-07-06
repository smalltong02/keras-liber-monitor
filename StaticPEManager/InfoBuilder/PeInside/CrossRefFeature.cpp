#include "CrossRefFeature.h"
#include "..\..\PeInfo.h"
#include "tinyxml2.h"
#include "stringutils.h"
#include "SharedFunc.h"

namespace cchips {
    bool CCrossRefFeatureBuilder::Scan(std::unique_ptr<cchips::PeFormat>& pe_format, std::unique_ptr<cchips::CRapidJsonWrapper>& json_result)
    {
        if (!pe_format || !json_result)
            return false;
        try {
			DetectParams params(DetectParams::SearchType::EXACT_MATCH, true, false);
			ToolInformation toolinfo;
			CompilerDetector compiler_detector(pe_format, params, toolinfo);
			auto result = compiler_detector.GetAllInformation();
			if (result == CompilerDetector::ReturnCode::return_ok) {
				for (const auto& m : toolinfo.errorMessages)
				{
					error_log("toolinfo error: %s", m.c_str());
				}
				auto& allocator = json_result->GetAllocator();
				std::unique_ptr<cchips::RapidValue> vtoolinfo = std::make_unique<cchips::RapidValue>();
				if (!vtoolinfo) return false;
				vtoolinfo->SetObject();
				if (toolinfo.detectedTools.size()) {
					for (const auto& m : toolinfo.detectedTools) {
						std::string info;
						if (m.name.length())
							info = m.name + ",";
						if (m.versionInfo.length())
							info.append(m.versionInfo + ",");
						if (m.additionalInfo.length())
							info.append(m.additionalInfo + ",");
						switch (m.type) {
						case DetectResult::ToolType::COMPILER:
						{
							if(info.length())
								vtoolinfo->AddMember("compiler", cchips::RapidValue(info.c_str(), allocator), allocator);
						}
						break;
						case DetectResult::ToolType::LINKER:
						{
							if (info.length())
								vtoolinfo->AddMember("linker", cchips::RapidValue(info.c_str(), allocator), allocator);
						}
						break;
						case DetectResult::ToolType::PACKER:
						{
							if (info.length())
								vtoolinfo->AddMember("packer", cchips::RapidValue(info.c_str(), allocator), allocator);
						}
						break;
						case DetectResult::ToolType::INSTALLER:
						{
							if (info.length())
								vtoolinfo->AddMember("installer", cchips::RapidValue(info.c_str(), allocator), allocator);
						}
						break;
						}
						
					}
				}
				if (toolinfo.detectedLanguages.size()) {
					for (const auto& l : toolinfo.detectedLanguages) {
						std::string info;
						if (l.name.length())
							info = l.name + ",";
						if (l.additionalInfo.length())
							info.append(l.additionalInfo + ",");
						if (info.length())
							vtoolinfo->AddMember("languages", cchips::RapidValue(info.c_str(), allocator), allocator);
					}
				}
				return json_result->AddTopMember("toolinfo", std::move(vtoolinfo));
			}

            return true;
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }

	const std::set<std::string> CompilerDetector::_external_database_suffixes = {
		".yar",
		".yara",
		".yarac"
	};

	template<typename I>
	std::string intToHexString(I w, bool addBase, unsigned fillToN)
	{
		static const char* digits = "0123456789abcdef";

		size_t hex_len = sizeof(I) << 1;

		std::string rc(hex_len, '0');
		for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
		{
			rc[i] = digits[(w >> j) & 0x0f];
		}

		bool started = false;
		std::string res;
		size_t j = 0;
		if (addBase)
		{
			res.resize(rc.size() + 2);
			res[0] = '0';
			res[1] = 'x';
			j = 2;
		}
		else
		{
			res.resize(rc.size());
		}
		for (size_t i = 0; i < rc.size(); ++i)
		{
			if (started)
			{
				res[j++] = rc[i];
			}
			else if (rc[i] != '0' || (rc.size() - i <= fillToN) || (i == rc.size() - 1))
			{
				res[j++] = rc[i];
				started = true;
			}
		}
		res.resize(j);

		return res;
	}

	template<typename N>
	bool strToNum(const std::string& str, N& number,
		std::ios_base& (*format)(std::ios_base&)) {
		std::istringstream strStream(str);
		N convNumber = 0;
		strStream >> format >> convNumber;
		if (strStream.fail() || !strStream.eof()) {
			return false;
		}

		// The above checks do not detect conversion of a negative number into an
		// unsigned integer. We have to perform an additional check here.
		if (std::is_unsigned<N>::value && str[0] == '-') {
			return false;
		}

		number = convNumber;
		return true;
	}

	CompilerDetector::CompilerDetector(
	std::unique_ptr<cchips::PeFormat>& pe_format,
	DetectParams& params,
	ToolInformation& toolInfo)
	: m_pe_format(pe_format)
	, m_detect_params(params)
	, m_toolinfo(toolInfo)
	, m_architecture(pe_format->getTargetArchitecture())
	, m_search(pe_format)
	, m_path_to_shared("") {
		
		if (!m_pe_format || !m_pe_format->IsValid()) {
			return;
		}

		m_external_suffixes = _external_database_suffixes;

		bool isFat = false;
		std::set<std::string> formats;
		std::set<std::string> archs;
		switch (FileDetector::DetectFileFormat(m_pe_format->getFileName())) {
			case FileDetector::file_format::format_elf:
			{
				break;
			}
			case FileDetector::file_format::format_pe:
			{
				m_heuristics = std::make_unique<PeHeuristics>(m_pe_format, m_search, m_toolinfo);
				formats.insert("pe");
				break;
			}
			case FileDetector::file_format::format_macho:
			{
				break;
			}
			case FileDetector::file_format::format_raw:
			{
				break;
			}
			case FileDetector::file_format::format_intel_hex:
			{
				break;
			}
			case FileDetector::file_format::format_coff:
			{
				break;
			}
			default:
				break;
		}

		auto bitWidth = m_pe_format->getBytesPerWord() * 8;

		if (!isFat) {
			switch (m_architecture)
			{
			case cchips::PeFormat::Architecture::X86_64:
				archs.insert("x64");
				[[fallthrough]];
			case cchips::PeFormat::Architecture::X86:
				archs.insert("x86");
				break;

			case cchips::PeFormat::Architecture::ARM:
				if (bitWidth == 32)
				{
					archs.insert("arm");
				}
				else
				{
					archs.insert("arm64");
				}
				break;

			case cchips::PeFormat::Architecture::MIPS:
				if (bitWidth == 32)
				{
					archs.insert("mips");
				}
				else
				{
					archs.insert("mips64");
				}
				break;

			case cchips::PeFormat::Architecture::POWERPC:
				if (bitWidth == 32)
				{
					archs.insert("ppc");
				}
				else
				{
					archs.insert("ppc64");
				}
				break;

			default:
				break;
			}
		}
		return;
	}
	
	CompilerDetector::ReturnCode CompilerDetector::GetAllInformation()
	{
		if (!m_pe_format || !m_pe_format->IsValid())
		{
			return ReturnCode::return_file_problem;
		}

		m_pe_format->getImageBaseAddress(m_toolinfo.imageBase);

		m_toolinfo.entryPointOffset = m_pe_format->getEpAddress(m_toolinfo.epAddress);
		if (m_toolinfo.entryPointOffset) {
			m_toolinfo.epAddress = m_toolinfo.imageBase + m_toolinfo.epAddress;
			m_toolinfo.entryPointAddress = true;
		}

		if ((m_toolinfo.overlaySize = m_pe_format->getOverlaySize()) != 0)
		{
			m_toolinfo.overlayOffset = m_pe_format->getOverlayOffset();
		}

		bool invalidEntryPoint = false;
		auto format = FileDetector::DetectFileFormat(m_pe_format->getFileName());
		if (format == FileDetector::file_format::format_pe)
		{
			// False EP offset (offset outside of file) doesn't have
			// to mean invalid EP as it can be memory only
			invalidEntryPoint = !m_toolinfo.entryPointAddress;
		}
		else
		{
			invalidEntryPoint = !m_toolinfo.entryPointAddress || !m_toolinfo.entryPointOffset;
		}

		if (!m_pe_format->getHexEpBytes(m_toolinfo.epBytes, m_detect_params.epBytesCount)
			&& !invalidEntryPoint)
		{
			return ReturnCode::return_file_problem;
		}

		auto epSec = m_pe_format->getEpSection();
		m_toolinfo.entryPointSection = (epSec != nullptr);
		if (epSec)
		{
			m_toolinfo.epSection = Section(*epSec);
		}

		auto status = getAllCompilers();
		if (invalidEntryPoint)
		{
			if (m_pe_format->isExecutable()
				|| m_toolinfo.entryPointAddress
				|| m_toolinfo.entryPointSection)
			{
				status = ReturnCode::return_entry_point_detection;
			}
		}

		return status;
	}

	bool CompilerDetector::getExternalDatabases()
	{
		auto result = false;

		// iterating over all files in directory
		for (auto& subpathIt : fs::directory_iterator("."))
		{
			auto subpath = subpathIt.path();
			if (fs::is_regular_file(subpath)
				&& std::any_of(m_external_suffixes.begin(), m_external_suffixes.end(),
					[&](const auto& suffix)
					{
						return endsWith(subpath.string(), suffix);
					}
				))
			{
				result = true;
				m_external_db.push_back(subpath.string());
			}
		}

		return result;
	}

	void CompilerDetector::removeCompilersWithLessSimilarity(double refRatio)
	{
		double actRatio;

		for (std::size_t i = 0, e = m_toolinfo.detectedTools.size(); i < e; ++i)
		{
			if (m_toolinfo.detectedTools[i].source == DetectResult::DetectionMethod::SIGNATURE)
			{
				actRatio = static_cast<double>(m_toolinfo.detectedTools[i].agreeCount)
					/ m_toolinfo.detectedTools[i].impCount;
				auto ratio = actRatio + std::numeric_limits<double>::epsilon()
					* std::abs(actRatio);
				if (ratio < refRatio)
				{
					m_toolinfo.detectedTools.erase(m_toolinfo.detectedTools.begin() + i);
					--i;
					--e;
				}
			}
		}
	}

	void CompilerDetector::removeUnusedCompilers()
	{
		std::size_t noOfCompilers = m_toolinfo.detectedTools.size();
		std::size_t lastBeneficial = 0;
		auto removeFlag = false;

		for (std::size_t i = 0; i < noOfCompilers; ++i)
		{
			if (m_toolinfo.isReliableResult(i))
			{
				lastBeneficial = i;
				removeFlag = true;
			}
		}

		if (removeFlag)
		{
			for (std::size_t i = lastBeneficial + 1; i < noOfCompilers; ++i)
			{
				if (m_toolinfo.detectedTools[i].source < DetectResult::DetectionMethod::SIGNATURE)
				{
					m_toolinfo.detectedTools.erase(m_toolinfo.detectedTools.begin() + i);
					--i;
					--noOfCompilers;
				}
			}
		}

		for (std::size_t i = 0; i < noOfCompilers; ++i)
		{
			const auto& first = m_toolinfo.detectedTools[i];

			for (std::size_t j = i + 1; j < noOfCompilers; ++j)
			{
				const auto& second = m_toolinfo.detectedTools[j];
				if (first.name == second.name
					&& (first.versionInfo == second.versionInfo
						|| second.versionInfo.empty())
					&& (first.additionalInfo == second.additionalInfo
						|| second.additionalInfo.empty()))
				{
					m_toolinfo.detectedTools.erase(m_toolinfo.detectedTools.begin() + j);
					--j;
					--noOfCompilers;
				}
			}
		}
	}

	void CompilerDetector::getAllHeuristics()
	{
		if (m_heuristics)
		{
			m_heuristics->getAllHeuristics();
		}
	}

	CompilerDetector::ReturnCode CompilerDetector::getAllSignatures()
	{
		return CompilerDetector::ReturnCode::return_unknown_cp;
	}

	bool CompilerDetector::compareExtraInfo(
		const DetectResult& a,
		const DetectResult& b,
		bool& result)
	{
		// Check by version
		if (!a.versionInfo.empty() && b.versionInfo.empty())
		{
			// Prefer detection with version
			result = true;
			return true;
		}
		if (a.versionInfo.empty() && !b.versionInfo.empty())
		{
			// Prefer detection with version
			result = false;
			return true;
		}

		// Check by extra info
		if (!a.additionalInfo.empty() && b.additionalInfo.empty())
		{
			// Prefer detection with extra info
			result = true;
			return true;
		}
		if (a.additionalInfo.empty() && !b.additionalInfo.empty())
		{
			// Prefer detection with extra info
			result = false;
			return true;
		}

		return false;
	}

	bool CompilerDetector::compareForSort(const DetectResult& a, const DetectResult& b)
	{
		if (a.strength == b.strength)
		{
			if (a.source == DetectResult::DetectionMethod::SIGNATURE && a.source == b.source)
			{
				// Equaly strong signature detections - check nibble counts
				const auto aRatio = static_cast<double>(a.agreeCount) / a.impCount;
				const auto bRatio = static_cast<double>(b.agreeCount) / b.impCount;
				if (areEqual(aRatio, bRatio))
				{
					if (isShorterPrefixOfCaseInsensitive(a.name, b.name)
						&& a.impCount == b.impCount)
					{
						// Decide by version or extra information
						bool compRes = false;
						return compareExtraInfo(a, b, compRes) ? compRes : false;
					}
					else
					{
						// Prefer bigger signature
						return a.impCount > b.impCount;
					}
				}
				else
				{
					// Prefer better match
					return aRatio > bRatio;
				}
			}

			// Everything is better than incomplete signature detection
			if (b.source == DetectResult::DetectionMethod::SIGNATURE
				&& b.agreeCount != b.impCount)
			{
				return true;
			}
			else if (a.source == DetectResult::DetectionMethod::SIGNATURE
				&& a.agreeCount != a.impCount)
			{
				return false;
			}

			// If both are same compilers with same detection strength
			if (isShorterPrefixOfCaseInsensitive(a.name, b.name))
			{
				// Decide by version or extra information
				bool compRes = false;
				if (compareExtraInfo(a, b, compRes))
				{
					return compRes;
				}
			}

			// Prefer heuristic
			return b.source == DetectResult::DetectionMethod::SIGNATURE;
		}

		// Prefer stronger method
		return a.strength > b.strength;
	}

	CompilerDetector::ReturnCode CompilerDetector::getAllCompilers()
	{
		const auto status = getAllSignatures();
		getAllHeuristics();
		std::stable_sort(
			m_toolinfo.detectedTools.begin(),
			m_toolinfo.detectedTools.end(),
			CompilerDetector::compareForSort);
		removeUnusedCompilers();
		if (m_toolinfo.detectedLanguages.empty())
		{
			for (const auto& item : m_toolinfo.detectedTools)
			{
				if (!item.isReliable())
				{
					continue;
				}

				const auto name = toLower(item.name);
				if (contains(name, ".net"))
				{
					m_toolinfo.addLanguage("CIL/.NET", "", true);
				}
			}
		}

		const bool isDetecteion = m_toolinfo.detectedTools.size()
			|| m_toolinfo.detectedLanguages.size();
		return status == CompilerDetector::ReturnCode::return_unknown_cp && isDetecteion
			? CompilerDetector::ReturnCode::return_ok
			: status;
	}

	void ToolInformation::addTool(
		DetectResult::DetectionMethod source,
		DetectResult::DetectionStrength strength,
		DetectResult::ToolType toolType,
		const std::string& name,
		const std::string& version,
		const std::string& extra)
	{
		DetectResult compiler;
		compiler.source = source;
		compiler.strength = strength;

		compiler.type = toolType;
		compiler.name = name;
		compiler.versionInfo = version;
		compiler.additionalInfo = extra;
		detectedTools.push_back(compiler);
	}

	void ToolInformation::addTool(
		std::size_t matchNibbles,
		std::size_t totalNibbles,
		DetectResult::ToolType toolType,
		const std::string& name,
		const std::string& version,
		const std::string& extra)
	{
		DetectResult compiler;
		compiler.source = DetectResult::DetectionMethod::SIGNATURE;
		compiler.strength = DetectResult::DetectionStrength::MEDIUM;
		compiler.impCount = totalNibbles;
		compiler.agreeCount = matchNibbles;

		// Compute strength
		if (totalNibbles != matchNibbles)
		{
			// Only partial match - very unreliable
			compiler.strength = DetectResult::DetectionStrength::LOW;
		}
		else if (matchNibbles > 32)
		{
			// We need at least 16B to consider signature reliable
			compiler.strength = DetectResult::DetectionStrength::HIGH;
		}

		compiler.type = toolType;
		compiler.name = name;
		compiler.versionInfo = version;
		compiler.additionalInfo = extra;
		detectedTools.push_back(compiler);
	}

	void ToolInformation::addLanguage(
		const std::string& name,
		const std::string& extra,
		bool bytecode)
	{
		// Prevent duplicates.
		for (auto& item : detectedLanguages)
		{
			if (item.name == name)
			{
				if (item.additionalInfo.empty() || item.additionalInfo == extra)
				{
					if (!item.bytecode)
					{
						item.bytecode = bytecode;
					}
					item.additionalInfo = extra;
					return;
				}
			}
		}

		DetectLanguage language;
		language.name = name;
		language.additionalInfo = extra;
		language.bytecode = bytecode;
		detectedLanguages.push_back(language);
	}

	bool ToolInformation::isReliableResult(std::size_t resultIndex) const
	{
		return resultIndex < detectedTools.size()
			&& detectedTools[resultIndex].isReliable();
	}

	bool ToolInformation::hasReliableResult() const
	{
		for (std::size_t i = 0, e = detectedTools.size(); i < e; ++i)
		{
			if (isReliableResult(i))
			{
				return true;
			}
		}

		return false;
	}

	ToolInformation::Packed ToolInformation::isPacked() const
	{
		bool detectedPacker = false;
		DetectResult::DetectionStrength strength = DetectResult::DetectionStrength::LOW;

		for (const auto& tool : detectedTools)
		{
			if (tool.isPacker())
			{
				detectedPacker = true;
				strength = strength > tool.strength ? strength : tool.strength;
			}
		}

		if (!detectedPacker)
		{
			/// @todo add entropy computation
			return Packed::PROBABLY_NO;
		}

		switch (strength)
		{
		case DetectResult::DetectionStrength::LOW:
			return Packed::PROBABLY_YES;

		case DetectResult::DetectionStrength::MEDIUM:
		case DetectResult::DetectionStrength::HIGH:
		case DetectResult::DetectionStrength::SURE:
			/* fall-thru */

		default:
			return Packed::PACKED;
		}
	}

	const std::map<PeFormat::Architecture, std::vector<Search::RelativeJump>> Search::RelativeJump::jumpMap =
	{
		{
			PeFormat::Architecture::X86,
			{Search::RelativeJump("EB", 1), Search::RelativeJump("E9", 4)}
		},
		{
			PeFormat::Architecture::X86_64,
			{Search::RelativeJump("EB", 1), Search::RelativeJump("E9", 4)}
		},
	};

	Search::RelativeJump::RelativeJump(
		std::string sSlash,
		std::size_t sBytesAfter)
		: slash(sSlash)
		, bytesAfter(sBytesAfter)
	{

	}

	std::string Search::RelativeJump::getSlash() const
	{
		return slash;
	}

	std::size_t Search::RelativeJump::getSlashNibbleSize() const
	{
		return slash.size();
	}

	std::size_t Search::RelativeJump::getBytesAfter() const
	{
		return bytesAfter;
	}

	Search::Search(std::unique_ptr<cchips::PeFormat>& pe_format)
		: m_pe_format(pe_format)
		, averageSlashLen(0)
	{
		if (!pe_format->getHexBytes(nibbles, 0, pe_format->getLoadedFileLength())) return;
		if (!pe_format->getStringBytes(plain, 0, pe_format->getLoadedFileLength())) return;
		fileLoaded = true;
		fileSupported = pe_format->hexToLittle(nibbles)
			&& pe_format->getNumberOfNibblesInByte();
		jumps = mapGetValueOrDefault(
			Search::RelativeJump::jumpMap,
			pe_format->getTargetArchitecture(),
			std::vector<RelativeJump>()
		);

		for (std::size_t i = 0, e = jumps.size(); i < e; ++i)
		{
			const auto len = jumps[i].getSlashNibbleSize();
			averageSlashLen += len;
			if (!len)
			{
				jumps.erase(jumps.begin() + i);
				--i;
				--e;
			}
		}

		if (averageSlashLen)
		{
			averageSlashLen /= jumps.size();
		}
	}

	bool Search::haveSlashes() const
	{
		return !jumps.empty();
	}

	std::size_t Search::nibblesFromBytes(std::size_t nBytes) const
	{
		return m_pe_format->nibblesFromBytes(nBytes);
	}

	std::size_t Search::bytesFromNibbles(std::size_t nNibbles) const
	{
		return m_pe_format->bytesFromNibbles(nNibbles);
	}

	const Search::RelativeJump* Search::getRelativeJump(
		std::size_t fileOffset,
		std::size_t shift,
		std::int64_t& moveSize) const
	{
		const auto nibbleOffset = nibblesFromBytes(fileOffset) + shift;
		moveSize = 0;

		for (const auto& jump : jumps)
		{
			const auto nibblesAfter = nibblesFromBytes(jump.getBytesAfter());
			if (!hasSubstringOnPosition(nibbles, jump.getSlash(), nibbleOffset)
				|| (nibbleOffset + jump.getSlashNibbleSize() + nibblesAfter - 1
					>= nibbles.length()))
			{
				continue;
			}

			std::uint64_t jumpedBytes = 0;
			if (!m_pe_format->getXByteOffset(
				fileOffset + bytesFromNibbles(jump.getSlashNibbleSize()),
				jump.getBytesAfter(),
				jumpedBytes,
				PeFormat::Endianness::LITTLE))
			{
				continue;
			}

			moveSize = static_cast<std::int64_t>(jumpedBytes);
			switch (jump.getBytesAfter())
			{
			case 1:
				moveSize = static_cast<std::int8_t>(moveSize);
				break;
			case 2:
				moveSize = static_cast<std::int16_t>(moveSize);
				break;
			case 4:
				moveSize = static_cast<std::int32_t>(moveSize);
				break;
			case 8:
				moveSize = static_cast<std::int64_t>(moveSize);
				break;
			default:
				assert(false && "Unexpected value of a switch expression");
			}

			moveSize = nibblesFromBytes(moveSize);
			return &jump;
		}

		return nullptr;
	}

	unsigned long long Search::countImpNibbles(const std::string& signPattern) const
	{
		unsigned long long count = 0;

		for (const auto& c : signPattern)
		{
			if (c == '/')
			{
				count += averageSlashLen;
			}
			else if (c != '-' && c != '?' && c != ';')
			{
				++count;
			}
		}

		return count;
	}

	unsigned long long Search::findUnslashedSignature(
		const std::string& signPattern,
		std::size_t startOffset,
		std::size_t stopOffset) const
	{
		if (startOffset > stopOffset)
		{
			return 0;
		}

		const auto startIterator = nibbles.begin() + nibblesFromBytes(startOffset);
		const auto stopIndex = nibblesFromBytes(stopOffset) + 1;
		const auto stopIterator = stopIndex < nibbles.size()
			? nibbles.begin() + stopIndex
			: nibbles.end();
		const auto it = std::search(
			startIterator,
			stopIterator,
			signPattern.begin(),
			signPattern.end(),
			[](const char fileNibble, const char signatureNibble)
			{
				return fileNibble == signatureNibble
					|| signatureNibble == '-'
					|| signatureNibble == '?'
					|| signatureNibble == ';';
			}
		);

		return (it != stopIterator) ? countImpNibbles(signPattern) : 0;
	}

	unsigned long long Search::findSlashedSignature(
		const std::string& signPattern,
		std::size_t startOffset,
		std::size_t stopOffset) const
	{
		if (startOffset > stopOffset)
		{
			return 0;
		}

		const auto areaSize = nibblesFromBytes(stopOffset - startOffset + 1);
		const auto signSize = signPattern.length()
			- std::count(signPattern.begin(), signPattern.end(), ';');
		if (areaSize < signSize)
		{
			return false;
		}
		const auto iters = startOffset == stopOffset ? 1 : areaSize - signSize + 1;

		for (std::size_t i = 0; i < iters; ++i)
		{
			const auto result = exactComparison(signPattern, startOffset, i);
			if (result)
			{
				return result;
			}
		}

		return 0;
	}

	unsigned long long Search::exactComparison(
		const std::string& signPattern,
		std::size_t fileOffset,
		std::size_t shift) const
	{
		for (std::size_t sigIndex = 0,
			fileIndex = nibblesFromBytes(fileOffset) + shift,
			fileLen = nibbles.length()
			;
			fileIndex < fileLen
			;
			++sigIndex, ++fileIndex)
		{
			if (sigIndex == signPattern.length() || signPattern[sigIndex] == ';')
			{
				return countImpNibbles(signPattern);
			}
			else if (signPattern[sigIndex] == '/')
			{
				std::int64_t moveSize = 0;
				const auto actShift = m_pe_format->getNumberOfNibblesInByte()
					? fileIndex % m_pe_format->getNumberOfNibblesInByte()
					: 0;
				const auto* jump = getRelativeJump(
					bytesFromNibbles(fileIndex), actShift, moveSize);
				if (!jump)
				{
					if (!haveSlashes())
					{
						--fileIndex;
						continue;
					}

					return 0;
				}

				// move after one nibble is in header of cycle
				fileIndex += jump->getSlashNibbleSize()
					+ nibblesFromBytes(jump->getBytesAfter())
					+ moveSize
					- 1;
			}
			else if (signPattern[sigIndex] != nibbles[fileIndex]
				&& signPattern[sigIndex] != '-'
				&& signPattern[sigIndex] != '?')
			{
				return 0;
			}
		}

		return 0;
	}

	bool Search::countSimilarity(
		const std::string& signPattern,
		Similarity& sim,
		std::size_t fileOffset,
		std::size_t shift) const
	{
		Similarity result;

		for (std::size_t sigIndex = 0,
			fileIndex = nibblesFromBytes(fileOffset) + shift,
			fileLen = nibbles.length()
			;
			fileIndex < fileLen
			;
			++sigIndex, ++fileIndex)
		{
			if (sigIndex == signPattern.length() || signPattern[sigIndex] == ';')
			{
				sim.same = result.same;
				sim.total = result.total;
				sim.ratio = static_cast<double>(result.same) / result.total;
				return countImpNibbles(signPattern);
			}
			else if (signPattern[sigIndex] == '-' || signPattern[sigIndex] == '?')
			{
				continue;
			}
			else if (signPattern[sigIndex] == '/')
			{
				std::int64_t moveSize = 0;
				const auto actShift = m_pe_format->getNumberOfNibblesInByte()
					? fileIndex % m_pe_format->getNumberOfNibblesInByte()
					: 0;
				const auto* jump = getRelativeJump(
					bytesFromNibbles(fileIndex), actShift, moveSize);
				if (!jump)
				{
					if (!haveSlashes())
					{
						--fileIndex;
						continue;
					}

					result.total += averageSlashLen;
				}
				else
				{
					result.total += jump->getSlashNibbleSize();
					result.same += jump->getSlashNibbleSize();
					fileIndex += jump->getSlashNibbleSize()
						+ nibblesFromBytes(jump->getBytesAfter())
						+ moveSize - 1;
				}
				continue;
			}
			else if (signPattern[sigIndex] == nibbles[fileIndex])
			{
				++result.same;
			}

			++result.total;
		}

		return false;
	}

	bool Search::areaSimilarity(
		const std::string& signPattern,
		Similarity& sim,
		std::size_t startOffset,
		std::size_t stopOffset) const
	{
		if (startOffset > stopOffset)
		{
			return false;
		}

		const auto areaSize = nibblesFromBytes(stopOffset - startOffset + 1);
		const auto signSize = signPattern.length()
			- std::count(signPattern.begin(), signPattern.end(), ';');
		if (areaSize < signSize)
		{
			return false;
		}
		const auto iters = startOffset == stopOffset
			? 1
			: areaSize - signSize + 1;
		auto result = false;
		Similarity act, max;

		for (std::size_t i = 0; i < iters; ++i)
		{
			if (countSimilarity(signPattern, act, startOffset, i)
				&& (act.ratio > max.ratio
					|| (areEqual(act.ratio, max.ratio)
						&& act.total > max.total)))
			{
				max.same = act.same;
				max.total = act.total;
				max.ratio = act.ratio;
				result = true;
			}
		}

		if (result)
		{
			sim.same = max.same;
			sim.total = max.total;
			sim.ratio = max.ratio;
		}

		return result;
	}

	bool Search::createSignature(
		std::string& pattern,
		std::size_t fileOffset,
		std::size_t size) const
	{
		pattern.clear();

		for (std::size_t i = 0,
			fileIndex = nibblesFromBytes(fileOffset),
			fileLen = nibbles.length(),
			nibbleSize = nibblesFromBytes(size)
			;
			fileIndex < fileLen && i < nibbleSize
			;
			++i, ++fileIndex)
		{
			std::int64_t moveSize = 0;
			const auto actShift = m_pe_format->getNumberOfNibblesInByte()
				? fileIndex % m_pe_format->getNumberOfNibblesInByte()
				: 0;
			const auto* jump = getRelativeJump(
				bytesFromNibbles(fileIndex), actShift, moveSize);
			if (jump)
			{
				pattern += '/';
				fileIndex += jump->getSlashNibbleSize()
					+ nibblesFromBytes(jump->getBytesAfter())
					+ moveSize
					- 1;
			}
			else
			{
				pattern += nibbles[fileIndex];
			}
		}

		pattern += ';';
		return isValidSignaturePattern(pattern);
	}

	const std::vector<std::pair<std::string, std::size_t>> Heuristics::delphiStrings =
	{
		{"for Win", 46},
		{"for Android", 48},
		{"for Mac OS X", 49},
		{"for Linux 64 bit", 53},
		{"Next Generation for iPhone Simulator", 73},
		{"Next Generation for iPhone ARM64", 69},
		{"Next Generation for iPhone", 63}
	};

	const std::map<std::string, std::string> Heuristics::delphiVersionMap =
	{
		{"32.0", "10.2 Tokyo"},
		{"31.0", "10.1 Berlin"},
		{"30.0", "10 Seattle"},
		{"29.0", "XE8"},
		{"28.0", "XE7"},
		{"27.0", "XE6"},
		{"26.0", "XE5"},
		{"25.0", "XE4"},
		{"24.0", "XE3"},
		{"23.0", "XE2"},
		{"22.0", "XE"},
	};

	Heuristics::Heuristics(
		std::unique_ptr<cchips::PeFormat>& peformat, 
		Search& searcher,
		ToolInformation& toolinfo)
		: pe_format(peformat)
		, search(searcher)
		, toolInfo(toolinfo)
	{
		canSearch = search.isFileLoaded() && search.isFileSupported();

		const auto secCounter = pe_format->getDeclaredNumberOfSections();
		sections.reserve(secCounter);
		for (std::size_t i = 0; i < secCounter; ++i)
		{
			const auto fsec = pe_format->getPeSection(i);
			if (fsec)
			{
				sections.push_back(fsec);

				// Add names to map
				auto secName = fsec->getName();
				if (!secName.empty()) {
					sectionNameMap[secName]++;
				}
			}
		}

		noOfSections = sections.size();
	}

	void Heuristics::getSectionHeuristics()
	{
		auto source = DetectResult::DetectionMethod::SECTION_TABLE_H;
		auto strength = DetectResult::DetectionStrength::HIGH;

		if (!noOfSections)
		{
			return;
		}

		// Compiler detections
		if (findSectionName(".go_export"))
		{
			addCompiler(source, strength, "gccgo");
			addPriorityLanguage("Go");
		}
		if (findSectionName(".note.go.buildid"))
		{
			addCompiler(source, strength, "gc");
			addPriorityLanguage("Go");
		}
		if (findSectionName(".gosymtab") || findSectionName(".gopclntab"))
		{
			addPriorityLanguage("Go");
		}
		if (findSectionName(".debug-ghc-link-info"))
		{
			addCompiler(source, strength, "GHC");
			addPriorityLanguage("Haskell");
		}
		if (findSectionName(".HP.init"))
		{
			addCompiler(source, strength, "HP C++");
			addLanguage("C++");
		}
	}

	bool Heuristics::parseGccComment(const std::string& record)
	{
		auto source = DetectResult::DetectionMethod::COMMENT_H;
		auto strength = DetectResult::DetectionStrength::LOW;

		const std::string prefix = "GCC: ";
		if (!startsWith(record, prefix))
		{
			return false;
		}

		static std::regex e("\\([^\\)]+\\)");
		std::string version = extractVersion(std::regex_replace(record, e, ""));
		if (!version.empty())
		{
			addCompiler(source, strength, "GCC", version);
			return true;
		}

		return false;
	}

	bool Heuristics::parseGhcComment(const std::string& record)
	{
		auto source = DetectResult::DetectionMethod::COMMENT_H;
		auto strength = DetectResult::DetectionStrength::LOW;

		if (record.size() < MINIMUM_GHC_RECORD_SIZE
			|| !startsWith(record, "GHC"))
		{
			return false;
		}

		const std::string version = record.substr(4);
		if (std::regex_match(
			version,
			std::regex("[[:digit:]]+.[[:digit:]]+.[[:digit:]]+")))
		{
			// Check for prior methods results
			if (isDetected("GHC"))
			{
				source = DetectResult::DetectionMethod::COMBINED;
				strength = DetectResult::DetectionStrength::HIGH;
			}

			addCompiler(source, strength, "GHC", version);
			addPriorityLanguage("Haskell");
			return true;
		}

		return false;
	}

	bool Heuristics::parseOpen64Comment(const std::string& record)
	{
		auto source = DetectResult::DetectionMethod::COMMENT_H;
		auto strength = DetectResult::DetectionStrength::LOW;

		const std::string prefix = "#Open64 Compiler Version ";
		const auto prefixLen = prefix.length();
		if (!startsWith(record, prefix))
		{
			return false;
		}

		const std::string separator = " : ";
		const auto separatorLen = separator.length();
		const auto pos = record.find(separator, prefixLen);
		if (pos == std::string::npos)
		{
			return false;
		}

		std::string additionalInfo;
		if (pos + separatorLen < record.length())
		{
			additionalInfo = record.substr(pos + separatorLen);
		}
		std::string version = record.substr(prefixLen, pos - prefixLen);
		addCompiler(source, strength, "Open64", version, additionalInfo);
		return true;
	}

	void Heuristics::getCommentSectionsHeuristics()
	{
		std::size_t nSecs = pe_format->getDeclaredNumberOfSections();
		for (std::size_t i = 0; i < nSecs; i++)
		{
			auto sec = pe_format->getPeSection(i);
			if (!sec)
			{
				continue;
			}
			const auto sec_name = sec->getName();
			if (!sec_name.empty() && (_stricmp(sec_name.c_str(), ".comment") == 0 || _stricmp(sec_name.c_str(), ".rdata") == 0))
			{
				std::string secContent;
				if (!sec || !sec->getString(secContent))
				{
					continue;
				}

				std::vector<std::string> records;
				separateStrings(secContent, records);

				for (const auto& item : records)
				{
					parseGccComment(item)
						|| parseGhcComment(item)
						|| parseOpen64Comment(item);
				}
			}
		}
	}

	bool Heuristics::parseGccProducer(const std::string& producer)
	{
		auto source = DetectResult::DetectionMethod::DWARF_DEBUG_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		const auto cpp = startsWith(producer, "GNU C++");
		const auto c = !cpp && startsWith(producer, "GNU C");
		const auto fortran = startsWith(producer, "GNU Fortran");
		if (!c && !cpp && !fortran)
		{
			return false;
		}

		std::string version = extractVersion(producer);

		addCompiler(source, strength, "GCC", version);
		addLanguage((c ? "C" : (cpp ? "C++" : "Fortran")));
		return true;
	}

	bool Heuristics::parseClangProducer(const std::string& producer)
	{
		auto source = DetectResult::DetectionMethod::DWARF_DEBUG_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		if (!contains(producer, "clang"))
		{
			return false;
		}

		std::string version = extractVersion(producer);
		addCompiler(source, strength, "LLVM", version);
		return true;
	}

	bool Heuristics::parseTmsProducer(const std::string& producer)
	{
		auto source = DetectResult::DetectionMethod::DWARF_DEBUG_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		if (!startsWith(producer, "TMS470 C/C++"))
		{
			return false;
		}

		std::string version = extractVersion(producer);
		addCompiler(
			source,
			strength,
			"Texas Instruments C/C++",
			version,
			"for TMS470");
		return true;
	}

	void Heuristics::getDwarfInfo()
	{
		return;
	}

	std::string Heuristics::commentSectionNameByFormat(unsigned int format)
	{
		switch (format) {
		case FileDetector::file_format::format_pe:
			return ".rdata";

		case FileDetector::file_format::format_elf:
			return ".comment";

		case FileDetector::file_format::format_macho:
			return "__comment";

		default:
			return std::string();
		}
	}

	std::string Heuristics::getEmbarcaderoVersion()
	{
		// Get comment section name
		auto sectionName = commentSectionNameByFormat((unsigned int)FileDetector::DetectFileFormat(pe_format->getFileName()));

		std::string content;
		const auto section = pe_format->getPeSection(sectionName);
		if (section && section->getString(content, 0, 0))
		{
			// Get offset to version in compiler ID string
			auto startOffset = content.find("Embarcadero Delphi ");
			if (startOffset != std::string::npos)
			{
				// Search for platform specific string
				std::string::size_type offset = startOffset + 19;
				for (const auto pair : delphiStrings)
				{
					offset = content.find(pair.first, startOffset);
					if (offset != std::string::npos)
					{
						offset = pair.second;
						break;
					}
				}

				// Search for compiler version in xx.x format
				if (offset != std::string::npos)
				{
					auto version = content.substr(startOffset + offset, 4);
					if (std::regex_match(
						version,
						std::regex("[[:digit:]]+.[[:digit:]]")))
					{
						return version;
					}
				}
			}
		}

		return std::string();
	}

	std::string Heuristics::embarcaderoVersionToExtra(const std::string& version)
	{
		auto pair = delphiVersionMap.find(version);
		if (pair != delphiVersionMap.end())
		{
			return pair->second;
		}

		return std::string();
	}

	void Heuristics::getEmbarcaderoHeuristics()
	{
		auto source = DetectResult::DetectionMethod::COMMENT_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		// Try to check for version in comment section
		auto version = getEmbarcaderoVersion();
		auto extra = embarcaderoVersionToExtra(version);

		// Special function often exported by Delphi XE5 and higher
		if (pe_format->getExport("TMethodImplementationIntercept"))
		{
			if (!version.empty())
			{
				// Increase detection strength
				source = DetectResult::DetectionMethod::COMBINED;
				strength = DetectResult::DetectionStrength::HIGH;
			}
			else
			{
				source = DetectResult::DetectionMethod::EXPORT_TABLE_H;
				strength = DetectResult::DetectionStrength::MEDIUM;

				version = "26.0+";
				extra = "XE5 or higher";
			}
		}

		if (!version.empty())
		{
			addCompiler(source, strength, "Embarcadero Delphi", version, extra);
			addPriorityLanguage("Delphi");
		}
	}

	void Heuristics::getSymbolHeuristic()
	{
		return;
	}

	void Heuristics::getCommonToolsHeuristics()
	{
		getSymbolHeuristic();
		getEmbarcaderoHeuristics();
		getSectionHeuristics();
		getDwarfInfo();
		getCommentSectionsHeuristics();
	}

	std::string Heuristics::getUpxVersion()
	{
		return "";
	}

	const DetectResult* Heuristics::isDetected(
		const std::string& name, const DetectResult::DetectionStrength minStrength)
	{
		for (const auto& detection : toolInfo.detectedTools)
		{
			if (startsWith(detection.name, name)
				&& detection.strength >= minStrength)
			{
				return &detection;
			}
		}

		return nullptr;
	}

	bool Signature::isValidSignaturePattern(const std::string& pattern)
	{
		const std::string bodyChars = "0123456789ABCDEF-\?/";
		const std::string allChars = bodyChars + ';';
		return isComposedOnlyOfChars(pattern, allChars)
			&& containsAnyOfChars(pattern, bodyChars)
			&& pattern.find(';') >= pattern.length() - 1;
	}

	const std::vector<PeHeuristics::PeHeaderStyle> PeHeuristics::headerStyles =
	{
		//	{"Unknown",      { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }},
			{"Microsoft",	 { 0x0090, 0x0003, 0x0000, 0x0004, 0x0000, 0xFFFF, 0x0000, 0x00B8, 0x0000, 0x0000, 0x0000, 0x0040, 0x0000 }},
			{"Borland",		 { 0x0050, 0x0002, 0x0000, 0x0004, 0x000F, 0xFFFF, 0x0000, 0x00B8, 0x0000, 0x0000, 0x0000, 0x0040, 0x001A }},
			{"PowerBasic",	 { 0x000A, 0x0002, 0x0000, 0x0004, 0x000F, 0xFFFF, 0x0000, 0x00C0, 0x0000, 0x0000, 0x0000, 0x0040, 0x0000 }},
			{"Watcom",		 { 0x0080, 0x0001, 0x0000, 0x0004, 0x0000, 0xFFFF, 0x0000, 0x00B8, 0x0000, 0x0000, 0x0000, 0x0040, 0x0000 }},
			{"IBM",			 { 0x0091, 0x0001, 0x0001, 0x0005, 0x0021, 0xFFFF, 0x0005, 0x0200, 0x68A3, 0x0000, 0x0003, 0x0040, 0x0000 }},
			//	{"Empty",        { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0040, 0x0000 }}
	};

	const std::vector<Signature> PeHeuristics::x86SlashedSignatures =
	{
		{"--MPRMMGVA--", "", "////////892504----00;", "", 0, 0},
		{"DotFix Nice Protect", "", "60BE--------8DBE--------5783CD--/619090505174--83C8--EB;", "", 0, 0},
		{"EXE Stealth", "1.1 - 2.7", "/60/E8000000005D81ED----40;", "", 0, 2},
		{"EXE Stealth", "1.1x", "/60/E8000000005D81ED--274000B91500000083C1;", "", 0, 2},
		{"EXE Stealth", "2.72 - 2.73", "/EB--536861726577617265202D20;", "", 0, 0},
		{"k.kryptor", "3", "/60E8--------5E8D----B9--------4933C05102----D3C04979--33----5900;", "", 0, 0},
		{"Morphnah", "1.0.7", "558BEC87E55D/558BEC83EC--5356576064A1--------8B40--8945--64A1--------C740;", "", 0, 0},
		{"NTSHELL", "5.0", "55E8--------5D81ED--------//8D85--------8DBD--------/8DB5--------/8BCF/2BC84FFD/33DB/8A07/D2C8/2A----/E8;", "", 0, 0},
		{"Obsidium", "", "/50/E8--------//33C0/71--//33C0/64FF30/648920EB;", "", 0, 0},
		{"Obsidium", "", "/E87892000039D7C7B6C7E8FDC1D910BAC96C682E58126D6928776F55A8990B0D4588C754028CC8979109F2B461633A28384037B805A98726CED01F92;", "", 0, 12},
		{"Obsidium", "", "/E8789200005CBF--------A5848CA3--------DC060020598B86--------5553AF0ECC8B4009AD832608BED00A596FD07C893E10A915150201584F0C;", "", 0, 12},
		{"Obsidium", "1.1.1.1 - 1.4.0.0 Beta", "/E8--------//8B------/83------------/33C0/C3;", "", 0, 12},
		{"Obsidium", "1.3.6.x", "/50/E8--000000//8B54240C/8382B8000000--/33C0/C3//33C0/64FF30/648920//8B00/C3/E9--000000/E8;", "", 0, 0},
		{"tElock", "0.60", "/60E8000000005883C008;", "", 0, 0},
		{"UPX", "", "/60BE--------8DBE--------5783CDFF/8B1E83EEFC11DB72EDB801;", "", 0, 0},
		{"yoda's Protector", "1.00", "558BEC53565760E8000000005D81ED--------E803000000/B9;", "", 0, 0},
		{"yoda's Protector", "1.01", "558BEC535657E803000000/E886000000E803000000/E879000000;", "", 0, 0},
		{"yoda's Protector", "1.02", "E803000000/BB55000000E803000000/E88F000000E803000000EB01--E882000000E803000000EB01--E8B8000000E803000000EB01--E8AB000000E803000000EB01--83FB55E803000000EB01--752EE803000000EB01--C360E8000000005D81ED233F42008BD581C2723F420052E801000000C3C3E803000000EB01--E80E000000E8D1FFFFFFC3E803000000EB01--33C064FF30648920CCC3E803000000EB01--33C064FF306489204BCCC3E803000000EB01--33DBB93A66420081E91D4042008BD581C21D4042008D3A8BF733C0E803000000EB01--E817000000909090E9C31F000033C064FF3064892043CCC3;", "Ashkbiz Danehkar", 0, 0},
		{"yoda's Protector", "1.03", "E803000000/BB55000000E803000000/E8--000000E803000000EB;", "Ashkbiz Danehkar", 0, 0},
		{"yoda's Protector", "1.03.1", "E803000000/BB55000000E803000000/E88F000000E803000000EB01--E882000000E803000000EB01--E8B8000000E803000000EB01--E8AB000000E803000000EB01--83FB55E803000000EB01--752EE803000000EB01--C360E8000000005D81ED747242008BD581C2C372420052E801000000C3C3E803000000EB01--E80E000000E8D1FFFFFFC3E803000000EB01--33C064FF30648920CCC3E803000000EB01--33C064FF306489204BCCC3E803000000EB01--33DBB93FA9420081E96E7342008BD581C26E7342008D3A8BF733C0E803000000EB01--E817000000909090E9982E000033C064FF3064892043CCC3;", "Ashkbiz Danehkar", 0, 0},
		{"yoda's Protector", "1.03.2", "E803000000/BB55000000E803000000/E88F000000E803000000EB01--E882000000E803000000EB01--E8B8000000E803000000EB01--E8AB000000E803000000EB01--83FB55E803000000EB01--752EE803000000EB01--C360E8000000005D81ED947342008BD581C2E373420052E801000000C3C3E803000000EB01--E80E000000E8D1FFFFFFC3E803000000EB01--33C064FF30648920CCC3E803000000EB01--33C064FF306489204BCCC3E803000000EB01--33DBB9BFA4420081E98E7442008BD581C28E7442008D3A8BF733C0E803000000EB01--E817000000909090E96329000033C064FF3064892043CCC3;", "Ashkbiz Danehkar", 0, 0},
		{"yoda's Protector", "1.03.3", "E803000000/BB55000000E803000000/E88E000000E803000000EB01--E881000000E803000000EB01--E8B7000000E803000000EB01--E8AA000000E803000000EB01--83FB55E803000000EB01--75;", "Ashkbiz Danehkar", 0, 0}
	};

	const std::map<char, std::string> PeHeuristics::peCompactMap =
	{
		{'=', "2.92.0"},
		{'F', "2.96.2"},
		{'M', "2.98.5"},
		{'T', "3.00.2"},
		{'X', "3.01.3"},
		{']', "3.02.1"},
		{'^', "3.02.2"},
		{'c', "3.03.5 (beta)"},
		{'g', "3.03.9 (beta)"},
		{'i', "3.03.10 (beta)"},
		{'n', "3.03.12 (beta)"},
		{'v', "3.03.18 (beta)"},
		{'w', "3.03.19 (beta)"},
		{'x', "3.03.20 (beta)"},
		{'y', "3.03.21 (beta)"},
		{'|', "3.03.23 (beta)"}
	};

	const std::vector<std::string> PeHeuristics::enigmaPatterns =
	{
		"60E8000000005D81ED--------81ED--------E9;",
		"//60E8000000005D81ED--------81ED--------E9;",
		"5051525355565741504151415241534154415541564157489C4881EC080000000FAE1C24E8000000005D;",
		"/83C4--/60E8000000005D81ED--------81ED--------E9;",
		"558BEC83C4--B8--------E8--------9A------------/60E8000000005D--ED;"
	};

	const std::vector<std::string> PeHeuristics::dotNetShrinkPatterns =
	{
		"20FE2B136028--------13--203B28136028--------13--11--11--161F4028--------26;",
		"20AD65133228--------13--206866133228--------13--11--11--161F4028--------26;",
		"20B9059F0728--------13--2066059F0728--------13--11--11--161F4028--------26;",
		"20E6EA19BE28--------13--2039EA19BE28--------13--11--11--161F4028--------26;"
	};

	const std::string PeHeuristics::msvcRuntimeString = "Microsoft Visual C++ Runtime Library";

	const std::vector<std::string> PeHeuristics::msvcRuntimeStrings =
	{
		msvcRuntimeString,
		toWide(msvcRuntimeString, 2),
		toWide(msvcRuntimeString, 4)
	};

	std::string PeHeuristics::getEnigmaVersion()
	{
		auto sec = pe_format->getPeSection(pe_format->getDeclaredNumberOfSections() - 1);
		if (!sec || !sec->getLoadedSize())
		{
			if(pe_format->getDeclaredNumberOfSections() > 1) 
			sec = (pe_format->getDeclaredNumberOfSections() > 1) ? pe_format->getPeSection(pe_format->getDeclaredNumberOfSections() - 2) : nullptr;
			if (!sec || !sec->getLoadedSize())
			{
				return "";
			}
		}

		const std::string pattern = "\0\0\0ENIGMA";
		const auto& content = search.getPlainString();
		const auto pos = content.find(pattern, sec->getOffset());
		if (pos < sec->getOffset() + sec->getLoadedSize())
		{
			std::uint64_t result1, result2;
			if (pe_format->getXByteOffset(pos + pattern.length(), 1, result1, PeFormat::Endianness::UNKNOWN)
				&& pe_format->getXByteOffset(
					pos + pattern.length() + 1, 1,
					result2, PeFormat::Endianness::UNKNOWN))
			{
				return std::to_string(result1) + "." + std::to_string(result2);
			}
		}

		return "";
	}

	std::string PeHeuristics::getUpxAdditionalInfo(std::size_t metadataPos)
	{
		const auto& content = search.getPlainString();

		std::string info;
		if (content.length() > metadataPos + 6)
		{
			switch (content[metadataPos + 6])
			{
			case 0x2:
			case 0x3:
			case 0x4:
				info += "[NRV2B]";
				break;

			case 0x5:
			case 0x6:
			case 0x7:
				info += "[NRV2D]";
				break;

			case 0x8:
			case 0x9:
			case 0xA:
				info += "[NRV2E]";
				break;

			case 0xE:
				info += "[LZMA]";
				break;

			default:
				break;
			}

			if (content.length() > metadataPos + 29)
			{
				info += info.empty() ? "" : " ";

				auto id = static_cast<std::uint32_t>(content[metadataPos + 28]);
				auto param = static_cast<std::uint32_t>(content[metadataPos + 29]);
				info += "[Filter: 0x" + intToHexString(id, false, 0)
					+ ", Param: 0x" + intToHexString(param, false, 0) + ']';
			}
		}

		return info;
	}

	void PeHeuristics::getGoHeuristics()
	{
		auto source = DetectResult::DetectionMethod::STRING_SEARCH_H;

		const auto section = pe_format->getPeSection(".text");
		if (!section)
		{
			return;
		}

		const auto goString = "\xFF Go build ID: ";
		if (section->getBytes(0, 15) == goString)
		{
			addCompiler(source, DetectResult::DetectionStrength::MEDIUM, "gc");
			addLanguage("Go");
		}
		else if (search.hasStringInSection(goString, section))
		{
			// Go build ID not on start of section
			addCompiler(source, DetectResult::DetectionStrength::LOW, "gc");
			addLanguage("Go");
		}
	}

	void PeHeuristics::getAutoItHeuristics()
	{
		auto source = DetectResult::DetectionMethod::STRING_SEARCH_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		std::string end;
		if (pe_format->getStringFromEnd(end, 8) && findAutoIt(end))
		{
			addCompiler(source, strength, "Aut2Exe");
			addPriorityLanguage("AutoIt", "", true);
		}

		const auto& content = search.getPlainString();
		const auto rsrc = pe_format->getPeSection(".rsrc");
		if (rsrc && rsrc->getOffset() < content.length()
			&& findAutoIt(content.substr(rsrc->getOffset())))
		{
			addCompiler(source, strength, "Aut2Exe");
			addPriorityLanguage("AutoIt", "", true);
		}
	}

	void PeHeuristics::getDotNetHeuristics()
	{
		if (pe_format->isDotNet() || pe_format->isPackedDotNet())
		{
			addLanguage("CIL/.NET", "", true);
		}
	}

	void PeHeuristics::getVisualBasicHeuristics()
	{
		auto source = DetectResult::DetectionMethod::LINKED_LIBRARIES_H;
		auto strength = DetectResult::DetectionStrength::HIGH;

		std::uint64_t version = 0;
		if (pe_format->isVisualBasic(version))
		{
			addCompiler(source, strength, "Visual Basic", std::to_string(version));
			addLanguage("Visual Basic");
		}
	}

	std::int32_t PeHeuristics::getInt32Unaligned(const std::uint8_t* codePtr)
	{
		const std::int32_t* codePtrInt32 = reinterpret_cast<const std::int32_t*>(
			codePtr
			);

		return codePtrInt32[0];
	}

	const std::uint8_t* PeHeuristics::skip_NOP_JMP8_JMP32(
		const std::uint8_t* codeBegin,
		const std::uint8_t* codePtr,
		const std::uint8_t* codeEnd,
		std::size_t maxCount)
	{
		while (codeBegin <= codePtr && codePtr < codeEnd && maxCount > 0)
		{
			signed long long movePtrBy = 0;

			switch (codePtr[0])
			{
			case 0x90:  // NOP, move by 1 byte
				movePtrBy = 1;
				break;

			case 0xEB:	// JMP rel8
				if ((codePtr + 2) > codeEnd || codePtr[1] == 0x80)
					return nullptr;
				movePtrBy = static_cast<int8_t>(codePtr[1]);
				break;

			case 0xE9:
				if ((codePtr + 5) > codeEnd
					|| getInt32Unaligned(codePtr + 1)
					== std::numeric_limits<std::int32_t>::min())
					return nullptr;
				movePtrBy = getInt32Unaligned(codePtr + 1);
				break;

			default:
				break;
			}

			// If no increment, it means there was no NOP/JMP and we are done
			if (movePtrBy == 0)
				break;

			// Check whether we got into the code range
			codePtr = codePtr + movePtrBy;
			maxCount--;
		}
		return codePtr;
	}

	void PeHeuristics::getHeaderStyleHeuristics()
	{
		const auto& content = search.getPlainString();

		// Must have at least IMAGE_DOS_HEADER
		if (content.length() > 0x40)
		{
			const char* e_cblp = content.c_str() + 0x02;

			for (size_t i = 0; i < headerStyles.size(); i++)
			{
				if (!std::memcmp(
					e_cblp,
					headerStyles[i].headerWords,
					sizeof(headerStyles[i].headerWords)))
				{
					addLinker(
						DetectResult::DetectionMethod::HEADER_H,
						DetectResult::DetectionStrength::MEDIUM,
						headerStyles[i].headerStyle);
				}
			}

		}
	}

	void PeHeuristics::getSlashedSignatures()
	{
		if (!(pe_format->getTargetArchitecture() == PeFormat::Architecture::X86 || pe_format->getTargetArchitecture() == PeFormat::Architecture::X86_64) || !canSearch || !toolInfo.entryPointOffset)
		{
			return;
		}

		const auto stopOffset = toolInfo.epOffset + LIGHTWEIGHT_FILE_SCAN_AREA;
		for (const auto& sig : x86SlashedSignatures)
		{
			auto start = toolInfo.epOffset;
			if (sig.startOffset != std::numeric_limits<unsigned>::max())
			{
				start += sig.startOffset;
			}

			auto end = stopOffset;
			if (sig.endOffset != std::numeric_limits<unsigned>::max())
			{
				end = std::min(
					stopOffset,
					toolInfo.epOffset
					+ sig.endOffset
					+ pe_format->bytesFromNibblesRounded(sig.pattern.length() - 1)
					- 1
				);
			}

			const auto nibbles = search.findSlashedSignature(sig.pattern, start, end);
			if (nibbles)
			{
				addPacker(nibbles, nibbles, sig.name, sig.version, sig.additional);
			}
		}
	}

	void PeHeuristics::getMorphineHeuristics()
	{
		if (!(pe_format->getTargetArchitecture() == PeFormat::Architecture::X86) || !toolInfo.entryPointOffset ||
			toolInfo.epOffset < 0x400 || toolInfo.epOffset > 0x1400)
		{
			return;
		}

		auto source = DetectResult::DetectionMethod::SECTION_TABLE_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		if (noOfSections > 2
			&& sections[0]->getName() == ".text"
			&& sections[1]->getName() == ".data"
			&& sections[2]->getName() == ".idata"
			&& sections[2]->getSizeInFile() == 0x200)
		{
			std::uint64_t rva, size;
			if (pe_format->getDataDirectoryRelative(1, rva, size) && size == 0x1000)
			{
				addPacker(source, strength, "Morphine", "1.2");
			}
		}
		else if (noOfSections > 1
			&& sections[0]->getName() == ".text"
			&& sections[1]->getName() == ".idata"
			&& (sections[1]->getSizeInFile() == 0x200
				|| sections[1]->getSizeInFile() == 0x400))
		{
			addPacker(source, strength, "Morphine", "2.7");
		}
	}

	void PeHeuristics::getStarForceHeuristics()
	{
		const auto epSection = toolInfo.epSection.getIndex();
		uint32_t e_lfanew = pe_format->getMzHeader().e_lfanew;
		uint16_t e_cblp = pe_format->getMzHeader().e_cblp;
		uint16_t e_cp = pe_format->getMzHeader().e_cp;

		if (noOfSections >= 4)
		{
			if (search.exactComparison("68--------FF25", toolInfo.epOffset)
				|| search.exactComparison("FF25", toolInfo.epOffset)
				|| search.exactComparison(
					"E8--------68ADDE0080FF15",
					toolInfo.epOffset))
			{
				// Version A
				if (!strncmp(sections[0]->getName().c_str(), ".sforce", 7)
					&& !strncmp(
						sections[epSection]->getName().c_str(),
						".start",
						6)
					&& (e_cblp == static_cast<uint16_t>(e_lfanew))
					&& (e_cp == 1))
				{
					addPacker(
						DetectResult::DetectionMethod::COMBINED,
						DetectResult::DetectionStrength::MEDIUM,
						"StarForce.A"
					);
					return;
				}

				// Version B
				if (sections[noOfSections - 1]->getName() == ".ps4"
					&& sections[noOfSections - 1]->getOffset() == loadedLength)
				{
					addPacker(
						DetectResult::DetectionMethod::COMBINED,
						DetectResult::DetectionStrength::MEDIUM,
						"StarForce.B"
					);
					return;
				}
			}

			if (sections[noOfSections - 1]->getName() == ".ps4"
				&& sections[noOfSections - 2]->getName() == ".discard"
				&& sections[noOfSections - 3]->getName() == ".rsrc")
			{
				addPacker(
					DetectResult::DetectionMethod::SECTION_TABLE_H,
					DetectResult::DetectionStrength::MEDIUM,
					"StarForce.C");
				return;
			}
		}

		if (13 <= noOfSections && noOfSections <= 15)
		{
			if (search.exactComparison(
				"FF25--------FF25--------FF25",
				toolInfo.epOffset))
			{
				if (sections[noOfSections - 1]->getName() == ".ps4")
				{
					if (sections[0]->getName() == ".text"
						&& sections[1]->getName() == ".data"
						&& sections[2]->getName() == ".data"
						&& sections[3]->getName() == ".share")
					{
						addPacker(
							DetectResult::DetectionMethod::COMBINED,
							DetectResult::DetectionStrength::MEDIUM,
							"StarForce.C");
						return;
					}
				}
			}
		}
	}

	void PeHeuristics::getSafeDiscHeuristics()
	{
		const auto& content = search.getPlainString();
		const std::string safeDiscString = "BoG_ *90.0&!!  Yy>";
		auto pos = content.find(safeDiscString, pe_format->getSizeOfHeaders() - 0x2C);

		if (pos == pe_format->getSizeOfHeaders() - 0x2C)
		{
			addPacker(
				DetectResult::DetectionMethod::SIGNATURE,
				DetectResult::DetectionStrength::MEDIUM,
				"SafeDisc");
		}
		else
		{
			for (std::size_t i = 0; i < noOfSections; i++)
			{
				// Note that original code does:
				// if (!strncmp((char *)pSection->Name, "stxt371\0", sizeof("stxt371\0")))
				if (sections[i]->getName() == "stxt371")
				{
					addPacker(
						DetectResult::DetectionMethod::SECTION_TABLE_H,
						DetectResult::DetectionStrength::MEDIUM,
						"SafeDisc");
					break;
				}

				// Note that original code does:
				// if (!strncmp((char *)pSection->Name, ".txt\0", sizeof(".txt\0"))
				// || !strncmp((char *)pSection->Name, ".txt2\0", sizeof(".txt2\0")))
				if (sections[i]->getName() == ".txt"
					|| sections[i]->getName() == ".txt2")
				{
					addPacker(
						DetectResult::DetectionMethod::SECTION_TABLE_H,
						DetectResult::DetectionStrength::LOW,
						"SafeDisc");
					break;
				}
			}
		}
	}

	bool PeHeuristics::checkSecuROMSignature(
		const char* fileData,
		const char* fileDataEnd,
		uint32_t fileOffset)
	{
		const uint32_t SecuRomMagic = 0x44646441;	// 'DddA'
		const char* header = fileData + fileOffset;
		uint32_t magicValue = 0;

		if (fileData <= header && (header + 0x08) <= fileDataEnd)
		{
			auto* secuRomHeader = reinterpret_cast<const uint32_t*>(header);

			if (secuRomHeader[1] == SecuRomMagic)
			{
				return true;
			}
			else if ((fileData + secuRomHeader[0] + sizeof(uint32_t)) < fileDataEnd)
			{
				memcpy(&magicValue, fileData + secuRomHeader[0], sizeof(uint32_t));
				if (magicValue == SecuRomMagic)
				{
					return true;
				}
			}
		}

		return false;
	}

	void PeHeuristics::getSecuROMHeuristics()
	{
		const char* fileData;
		uint32_t SecuromOffs = 0;
		bool foundSecuROM = false;

		// There must be at least 0x2000 extra bytes beyond the last section,
		// last data directory, last debug directory and digital signature.
		if (pe_format->getOverlaySize() >= 0x2000)
		{
			// The entire file must be loaded to memory
			if (loadedLength >= declaredLength)
			{
				// Retrieve the offset of the securom header
				fileData = search.getPlainString().c_str();
				memcpy(
					&SecuromOffs,
					fileData + loadedLength - sizeof(uint32_t),
					sizeof(uint32_t));

				// Verify it
				if (checkSecuROMSignature(
					fileData,
					fileData + loadedLength,
					SecuromOffs - sizeof(uint32_t)))
				{
					foundSecuROM = true;
				}
				if (checkSecuROMSignature(
					fileData,
					fileData + loadedLength,
					loadedLength - (SecuromOffs - 0x0C)))
				{
					foundSecuROM = true;
				}

				if (foundSecuROM)
				{
					addPacker(
						DetectResult::DetectionMethod::STRING_SEARCH_H,
						DetectResult::DetectionStrength::HIGH,
						"SecuROM"
					);
				}
			}
		}
	}

	void PeHeuristics::getMPRMMGVAHeuristics()
	{
		const auto& content = search.getPlainString();
		const uint8_t* fileData = reinterpret_cast<const uint8_t*>(
			content.c_str());
		const uint8_t* filePtr = fileData + toolInfo.epOffset;
		const uint8_t* fileEnd = fileData + content.length();
		unsigned long long offset1;

		// Skip up to 8 NOPs or JMPs
		filePtr = skip_NOP_JMP8_JMP32(fileData, filePtr, fileEnd, 8);
		if (filePtr != nullptr)
		{
			if (search.exactComparison("892504----00", toolInfo.epOffset))
			{
				offset1 = search.findUnslashedSignature(
					"64FF3500000000",
					toolInfo.epOffset,
					toolInfo.epOffset + 0x80);
				if (offset1 != 0)
				{
					if (search.findUnslashedSignature(
						"64892500000000",
						toolInfo.epOffset + offset1,
						toolInfo.epOffset + offset1 + 0x40))
					{
						addPacker(
							DetectResult::DetectionMethod::STRING_SEARCH_H,
							DetectResult::DetectionStrength::HIGH,
							"MPRMMGVA");
					}
				}
			}
		}
	}

	void PeHeuristics::getActiveMarkHeuristics()
	{
		if (search.exactComparison(
			"00544D53414D564F48A49BFDFF2624E9D7F1D6F0D6AEBEFCD6DFB5C1D01F07CE",
			toolInfo.overlayOffset))
		{
			addPacker(
				DetectResult::DetectionMethod::STRING_SEARCH_H,
				DetectResult::DetectionStrength::HIGH,
				"ActiveMark"
			);
		}
	}

	void PeHeuristics::getRLPackHeuristics()
	{
		if (search.exactComparison(
			"57C7C772AFB4DF8D3D5FBA581AFFCF0FACF7F20FBDFEF7C75CDC30270FBAF7330FBBF70FCFBF64A909DB85F681DFAC194648F7DF0FA3F7C7C741BC79A085F7D1",
			toolInfo.epOffset))
		{
			addPacker(
				DetectResult::DetectionMethod::STRING_SEARCH_H,
				DetectResult::DetectionStrength::HIGH, "RLPack"
			);
		}
	}

	void PeHeuristics::getPetiteHeuristics()
	{
		if (search.exactComparison("B8--------608DA800----FF68--------6A40680030000068------006A00FF90----00008944241CBB--0300008DB5------008BF850E80A00000074078B44", toolInfo.epOffset) ||
			search.exactComparison("B8--------669C60508D905C0100006800004000833A000F84--------8B04248B0A0FBAF11F7313FD8BF08BF8037204037A08F3A583C20CFCEBD983C210", toolInfo.epOffset) ||
			search.exactComparison("B8--------669C60508BD8030068--------6A00FF50148BCC8DA054BC0000508BC38D9010160000680000----51508004240850800424425080042461508004", toolInfo.epOffset) ||
			search.exactComparison("B8--------669C60508BD8030068--------6A00FF501C894308680000----8B3C248B336681C780078D741E08893B538B5E10B880080000566A0250576A--6A", toolInfo.epOffset) ||
			search.exactComparison("B8--------669C60508BD8030068--------6A00FF501C8943088BC303006870BC00006A00FF501C8BCC8DA070BC000089612E536800004000518B7C24048B33", toolInfo.epOffset) ||
			search.exactComparison("B8--------669C60508D8800----008D90----00008BDC8BE1680000----53508004240850800424425080042461508004249D50800424BB833A000F84DC1400", toolInfo.epOffset) ||
			search.exactComparison("B8--------68--------64FF350000000064892500000000669C6050", toolInfo.epOffset) ||
			search.exactComparison("B8--------6A--68--------64FF350000000064892500000000669C60508BD8030068--------6A00FF50", toolInfo.epOffset))
		{
			addPacker(
				DetectResult::DetectionMethod::STRING_SEARCH_H,
				DetectResult::DetectionStrength::HIGH,
				"Petite"
			);
		}
	}

	void PeHeuristics::getPelockHeuristics()
	{
		std::uint64_t rva, size;
		if (pe_format->getDataDirectoryRelative(1, rva, size)
			&& size == 0x5C
			&& pe_format->getDataDirectoryRelative(15, rva, size)
			&& size == 0x1000)
		{
			addPacker(
				DetectResult::DetectionMethod::OTHER_H,
				DetectResult::DetectionStrength::MEDIUM,
				"PELock",
				"1.x"
			);
		}
	}

	void PeHeuristics::getEzirizReactorHeuristics()
	{
		auto source = DetectResult::DetectionMethod::COMBINED;
		auto strength = DetectResult::DetectionStrength::HIGH;
		std::string version;

		if (canSearch)
		{
			const auto sec0 = pe_format->getPeSection((unsigned long long)(0));
			const auto sec1 = pe_format->getPeSection(1);

			if (sec0
				&& search.findUnslashedSignature(
					"558BECB90F0000006A006A004975F951535657B8--------E8;",
					sec0->getOffset(),
					sec0->getOffset() + sec0->getLoadedSize() - 1))
			{
				version = "3.X";
			}
			else if (sec1
				&& sec1->getPeCoffFlags() == 0xC0000040
				&& search.findUnslashedSignature(
					"5266686E204D182276B5331112330C6D0A204D18229EA129611C76B505190158;",
					sec1->getOffset(),
					sec1->getOffset() + sec1->getLoadedSize() - 1))
			{
				version = "4.0.0.0 - 6.0.0.0";
			}
			else if (noOfSections >= 4
				&& toolInfo.entryPointOffset
				&& findSectionName(".text") == 1
				&& findSectionName(".rsrc") == 1)
			{
				const auto& resTable = pe_format->getResourceTable();
				const auto* resVer = pe_format->getVersionResource();
				if (resTable && resTable->hasResourceWithName("__") && resVer)
				{
					std::size_t lId;
					if (resVer->getLanguageId(lId) && !lId)
					{
						if (search.exactComparison(
							"E8--------E9--------6A0C68;",
							toolInfo.epOffset))
						{
							version = "4.0.0.0 - 4.4.7.5";
						}
						else if (search.exactComparison(
							"E8--------E9--------8BFF558BEC83EC208B45085657;",
							toolInfo.epOffset))
						{
							version = "4.5.0.0 - 6.2.9.2";
						}
					}
				}
			}
		}

		if (!version.empty())
		{
			addPacker(source, strength, "Eziriz .NET Reactor", version);
			addLanguage("CIL/.NET", "", true);
		}
	}

	void PeHeuristics::getUpxHeuristics()
	{
		auto source = DetectResult::DetectionMethod::STRING_SEARCH_H;
		auto strength = DetectResult::DetectionStrength::HIGH;

		// UPX 1.00 - UPX 1.07
		// format: UPX 1.0x
		const std::string upxVer = "UPX 1.0";
		const auto& content = search.getPlainString();
		auto pos = content.find(upxVer);
		if (pos < 0x500 && pos < content.length() - upxVer.length())
		{
			// we must decide between UPX and UPX$HiT
			source = DetectResult::DetectionMethod::COMBINED;
			if (noOfSections == 3
				&& sections[0]->getName() == ".code"
				&& sections[1]->getName() == ".text"
				&& sections[2]->getName() == ".rsrc")
			{
				addPacker(source, strength, "UPX", "UPX$HiT");
			}
			else
			{
				const std::string versionPrefix = "1.0";
				addPacker(
					source,
					strength,
					"UPX",
					versionPrefix + content[pos + upxVer.length()]
				);
			}

			return;
		}

		// UPX 1.08 and later
		// format: x.xx'\0'UPX!
		const std::size_t minPos = 5, verLen = 4;
		pos = content.find("UPX!");
		if (pos >= minPos && pos < 0x500 && !sections.empty() && pos < sections[0]->getOffset())
		{
			std::string version;
			std::size_t num;
			if (strToNum(content.substr(pos - minPos, 1), num, std::dec)
				&& strToNum(content.substr(pos - minPos + 2, 2), num, std::dec))
			{
				version = content.substr(pos - minPos, verLen);
			}
			std::string additionalInfo = getUpxAdditionalInfo(pos);
			if (!additionalInfo.empty())
			{
				if (!version.empty())
				{
					version += ' ';
				}
				version += additionalInfo;
			}
			addPacker(source, strength, "UPX", version);
		}
	}

	void PeHeuristics::getFsgHeuristics()
	{
		auto source = DetectResult::DetectionMethod::STRING_SEARCH_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		if (search.hasString(
			"FSG!",
			pe_format->getPeHeaderOffset(),
			pe_format->getMzHeaderSize()))
		{
			addPacker(source, strength, "FSG");
		}
	}

	void PeHeuristics::getPeCompactHeuristics()
	{
		auto source = DetectResult::DetectionMethod::STRING_SEARCH_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		// format: PEC2[any character]O
		const std::string pattern = "PEC2";
		const auto patLen = pattern.length();

		const auto& content = search.getPlainString();
		const auto pos = content.find(pattern);

		if (pos < 0x500
			&& pos + patLen + 2 <= content.length()
			&& content[pos + patLen + 1] == 'O')
		{
			for (const auto& item : peCompactMap)
			{
				if (content[pos + patLen] == item.first)
				{
					addPacker(source, strength, "PECompact", item.second);
					return;
				}
			}

			addPacker(source, strength, "PECompact");
		}

		if (search.hasString("PECompact2", 0, 0x4FF))
		{
			addPacker(source, strength, "PECompact", "2.xx - 3.xx");
		}
	}

	void PeHeuristics::getAndpakkHeuristics()
	{
		auto source = DetectResult::DetectionMethod::COMBINED;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		if (noOfSections == 1 && sections[0]->getName() == "ANDpakk2")
		{
			const auto& table = pe_format->getImportTable();
			if (table && table->getNumberOfLibraries() == 1)
			{
				addPacker(source, strength, "ANDpakk", "2.x");
			}
		}
	}

	void PeHeuristics::getEnigmaHeuristics()
	{
		auto source = DetectResult::DetectionMethod::COMBINED;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		if (canSearch && toolInfo.entryPointOffset
			&& search.exactComparison(
				"60E8000000005D83----81ED;",
				toolInfo.epOffset))
		{
			const auto sec = pe_format->getPeSection(".data");
			if (sec)
			{
				const std::string pattern = "Enigma protector v";
				const auto& content = search.getPlainString();
				const auto pos = content.find(pattern, sec->getOffset());
				if (pos < sec->getOffset() + sec->getSizeInFile()
					&& pos <= content.length() - 4)
				{
					addPacker(
						source,
						strength,
						"Enigma",
						content.substr(pos + pattern.length(), 4)
					);
					return;
				}
			}
		}

		const auto& importTable = pe_format->getImportTable();
		if (importTable && importTable->getNumberOfImportsInLibrary(1) == 1)
		{
			const auto* import = importTable->getImport("MessageBoxA");
			if (import && import->getLibraryIndex() == 1)
			{
				const auto version = getEnigmaVersion();
				if (!version.empty())
				{
					addPacker(source, strength, "Enigma", version);
					return;
				}
			}
		}

		for (const auto& sign : enigmaPatterns)
		{
			if (canSearch && toolInfo.entryPointOffset
				&& search.exactComparison(sign, toolInfo.epOffset))
			{
				addPacker(
					DetectResult::DetectionMethod::SIGNATURE,
					strength,
					"Enigma",
					getEnigmaVersion());
				return;
			}
		}

		if (pe_format->isDotNet()
			&& search.hasStringInSection("\0\0\0ENIGMA", std::size_t(0)))
		{
			addPacker(DetectResult::DetectionMethod::SIGNATURE, strength, "Enigma");
			return;
		}
	}

	void PeHeuristics::getVBoxHeuristics()
	{
		auto source = DetectResult::DetectionMethod::SIGNATURE;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		const std::string sig =
			"FF7424--FF7424--FF7424--68--------68--------68--------68--------FF15--------68--------FFD0C2;";
		if (canSearch && toolInfo.entryPointOffset
			&& search.exactComparison(sig, toolInfo.epOffset))
		{
			std::string version;
			const auto& table = pe_format->getImportTable();
			if (table && table->hasLibrary("vboxp410.dll"))
			{
				source = DetectResult::DetectionMethod::LINKED_LIBRARIES_H;
				strength = DetectResult::DetectionStrength::HIGH;
				version = "4.10";
			}

			addPacker(source, strength, "VBox", version);
		}
	}

	void PeHeuristics::getActiveDeliveryHeuristics()
	{
		auto source = DetectResult::DetectionMethod::SIGNATURE;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		const std::string sig =
			"64A1--------558BEC6A--68--------68--------50648925--------83EC605356578965--FF15;";
		if (canSearch && toolInfo.entryPointOffset
			&& pe_format->getPeSection("actdlvry")
			&& search.exactComparison(sig, toolInfo.epOffset))
		{
			addPacker(source, strength, "Active Delivery");
		}
	}

	void PeHeuristics::getAdeptProtectorHeuristics()
	{
		auto source = DetectResult::DetectionMethod::STRING_SEARCH_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		if (pe_format->isDotNet()
			&& search.hasStringInSection("ByAdeptProtector", std::size_t(0)))
		{
			std::string version;
			if (search.hasStringInSection("STAThreadAttribute", std::size_t(0)))
			{
				version = "2.1";
			}
			addPacker(source, strength, "Adept Protector", version);
		}
	}

	void PeHeuristics::getCodeLockHeuristics()
	{
		auto source = DetectResult::DetectionMethod::LINKED_LIBRARIES_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		const auto& table = pe_format->getImportTable();
		if (table && table->hasLibrary("CODE-LOCK.OCX"))
		{
			addPacker(source, strength, "Code-Lock");
		}
	}

	void PeHeuristics::getNetHeuristic()
	{
		auto source = DetectResult::DetectionMethod::STRING_SEARCH_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		if (!pe_format->isDotNet() || !noOfSections)
		{
			return;
		}

		// normal string search
		std::size_t idx = 0;
		if (search.hasStringInSection(
			"Protected_By_Attribute\0NETSpider.Attribute",
			idx))
		{
			addPacker(source, strength, ".NET Spider", "0.5 - 1.3");
		}
		if (search.hasStringInSection(
			"Protected/Packed with ReNET-Pack by stx",
			idx))
		{
			addPacker(source, strength, "ReNET-pack");
		}
		if (search.hasStringInSection("\0NetzStarter\0netz\0", idx))
		{
			addPacker(source, strength, ".NETZ");
		}

		// unslashed signatures
		const auto& sec = pe_format->getPeSection((unsigned long long)0);
		if (canSearch && sec)
		{
			std::string version;
			const auto start = sec->getOffset();
			const auto end = start + sec->getLoadedSize() - 1;

			const std::string sig =
				"0000010B160C----------0208----------0D0906085961D21304091E630861D21305070811051E62110460D19D081758;";
			if (search.findUnslashedSignature(sig, start, end))
			{
				version = "1.7 - 1.8";
			}
			else if (search.hasStringInSection("?.resources", sec))
			{
				version = "1.x";
			}
			if (!version.empty())
			{
				addPacker(source, strength, "Phoenix", version);
			}

			if (search.findUnslashedSignature(
				"282D00000A6F2E00000A14146F2F00000A;",
				start,
				end))
			{
				addPacker(source, strength, "AssemblyInvoke");
			}

			if (search.findUnslashedSignature(
				"436C69005300650063007500720065;",
				start,
				end))
			{
				addPacker(source, strength, "CliSecure");
			}

			// Note: Before modifying the following loop to std::any_of(),
			//       please see #231 (compilation bug with GCC 5).
			for (const auto& str : dotNetShrinkPatterns)
			{
				if (search.findUnslashedSignature(str, start, end))
				{
					addPacker(source, strength, ".netshrink", "2.01 (demo)");
					break;
				}
			}
		}
	}

	void PeHeuristics::getExcelsiorHeuristics()
	{
		auto source = DetectResult::DetectionMethod::COMBINED;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		const std::string sig =
			"83EC--53555657E8--------6A--5B391D--------8BF37E--8B3D--------A1--------8B----8A08;";
		if (canSearch && toolInfo.entryPointOffset
			&& search.exactComparison(sig, toolInfo.epOffset)
			&& search.hasString("ExcelsiorII1", declaredLength, loadedLength - 1))
		{
			addInstaller(source, strength, "Excelsior Installer");
		}
	}

	void PeHeuristics::getVmProtectHeuristics()
	{
		static const std::size_t BLOCK_COUNT = 64;
		static const std::size_t BLOCK_SIZE = BLOCK_COUNT * sizeof(std::uint32_t);
		auto source = DetectResult::DetectionMethod::COMBINED;
		auto strength = DetectResult::DetectionStrength::HIGH;

		if (noOfSections < 3 || (noOfSections == 3 && !sections[0]->getSizeInFile()))
		{
			return;
		}

		for (std::size_t i = 0; i < noOfSections; ++i) {
			auto& section = sections[i];
			if (section->isCode() && section->getSizeInFile() > BLOCK_SIZE)
			{
				std::uint32_t checksum = 0;
				// Compute the sum of first 64 DWORDs of the executable section.
				for (std::size_t i = 0; i < BLOCK_COUNT; ++i)
				{
					checksum += section->getBytesAtOffsetAsNumber<std::uint32_t>(
						i * sizeof(std::uint32_t)
					);
				}
				// If the checksum is correct, the sample is 100% packed with
				// VMProtect 2.04+.
				if (checksum == 0xB7896EB5)
				{
					addPacker(source, strength, "VMProtect", "2.04+");
					return;
				}
			}
		}

		if (toolInfo.entryPointOffset && canSearch)
		{
			const std::string sig = "54C70424--------9C60C74424----------C64424----887424--60;";
			if (search.exactComparison(sig, toolInfo.epOffset))
			{
				addPacker(source, strength, "VMProtect", "2.06");
				return;
			}
			else if (pe_format->getPeSection(".vmp0")
				&& (search.exactComparison("68--------E9;", toolInfo.epOffset)
					|| search.exactComparison("68--------E8;", toolInfo.epOffset)))
			{
				addPacker(source, strength, "VMProtect", "1.60 - 2.05");
				return;
			}
		}

		for (const std::string secName : {".vmp0", ".vmp1", ".vmp2"})
		{
			if (pe_format->getPeSection(secName))
			{
				addPacker(source, strength, "VMProtect");
				return;
			}
		}
	}

	void PeHeuristics::getBorlandDelphiHeuristics()
	{
		auto source = DetectResult::DetectionMethod::COMBINED;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		std::uint64_t imageBaseAddr;
		if (!pe_format->getImageBaseAddress(imageBaseAddr)
			|| !toolInfo.entryPointSection
			|| toolInfo.epSection.getIndex()
			|| toolInfo.epSection.getOffset() != 0x400
			|| toolInfo.epSection.getAddress() < imageBaseAddr
			|| toolInfo.epSection.getAddress() - imageBaseAddr != 0x1000
			|| toolInfo.epSection.getName() != "CODE"
			|| noOfSections < 8
			|| sections[1]->getName() != "DATA"
			|| sections[2]->getName() != "BSS"
			|| sections[3]->getName() != ".idata"
			|| sections[4]->getName() != ".tls"
			|| sections[5]->getName() != ".rdata"
			|| sections[6]->getName() != ".reloc"
			|| sections[7]->getName() != ".rsrc")
		{
			return;
		}

		const std::string str = "SOFTWARE\\Borland\\Delphi\\RTL\0FPUMaskValue";
		if (search.hasStringInSection(str, sections[0])
			|| pe_format->getTimeStamp() == 0x2A425E19) // 1992-06-19
		{
			addCompiler(source, strength, "Borland Delphi");
		}
	}

	void PeHeuristics::getBeRoHeuristics()
	{
		auto source = DetectResult::DetectionMethod::STRING_SEARCH_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		const std::string& str = "Compiled by: BeRoTinyPascal - (C) Copyright 2006, Benjamin";
		if (toolInfo.entryPointSection
			&& search.hasStringInSection(str, toolInfo.epSection.getIndex()))
		{
			addCompiler(source, strength, "BeRo Tiny Pascal");
			addLanguage("Pascal");
		}
	}

	void PeHeuristics::getMsvcIntelHeuristics()
	{
		auto source = DetectResult::DetectionMethod::COMBINED;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		if (std::none_of(msvcRuntimeStrings.begin(), msvcRuntimeStrings.end(),
			[this](const auto& str)
			{
				return this->search.hasStringInSection(str, ".rdata");
			}
		))
		{
			return;
		}

		if (findSectionName(".reloc") == 1)
		{
			addCompiler(source, strength, "MSVC");
			addPriorityLanguage("C++");
			return;
		}
		else if (findSectionName(".data1") == 1)
		{
			addCompiler(source, strength, "Intel XE");
		}
	}

	void PeHeuristics::getArmadilloHeuristic()
	{
		auto majorVersion = pe_format->getMajorLinkerVersion();
		auto minorVersion = pe_format->getMinorLinkerVersion();

		if (majorVersion == 'S' && minorVersion == 'R')
		{
			// Note: do NOT perform any extra checks here (like sections named
			// "PDATA000"). They are often not present in the image at all.
			// Not even Windows 10's ntdll.dll (LdrpIsImageArmadilloProtected)
			// checks for them.
			addPacker(
				DetectResult::DetectionMethod::LINKER_VERSION_H,
				DetectResult::DetectionStrength::HIGH,
				"Armadillo"
			);
		}
	}

	void PeHeuristics::getStarforceHeuristic()
	{
		auto source = DetectResult::DetectionMethod::SECTION_TABLE_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		if (noOfSections < 2)
		{
			return;
		}

		// Check import table for protect.dll library
		const auto& importTab = pe_format->getImportTable();
		if (importTab && importTab->hasLibraryCaseInsensitive("protect.dll"))
		{
			strength = DetectResult::DetectionStrength::HIGH;

			// This section name seems to appear in other binaries
			// too so check only if protect.dll is linked
			if (findSectionName(".ps4") > 0)
			{
				addPacker(source, strength, "StarForce", "4.x - 5.x");
				return;
			}
		}

		const auto first = sections[0]->getName();
		if (startsWith(first, ".sforce") || findSectionName(".brick") > 0)
		{
			std::string version;
			if (first == ".sforce3")
			{
				version = "3.x";
			}

			addPacker(source, strength, "StarForce", version);
		}
	}

	void PeHeuristics::getLinkerVersionHeuristic()
	{
		//if (!pe_format->getRichHeader())
		//{
		//	// Rich header control was previously removed but there
		//	// are apparently other linkers with same versions
		//	return;
		//}

		auto source = DetectResult::DetectionMethod::COMBINED;
		auto strength = DetectResult::DetectionStrength::HIGH;

		auto majorVersion = pe_format->getMajorLinkerVersion();
		auto minorVersion = pe_format->getMinorLinkerVersion();

		// Source: https://en.wikipedia.org/wiki/Microsoft_Visual_C++
		// Source: https://en.wikipedia.org/wiki/Microsoft_Visual_Studio

		std::string studioVersion, linkerVersion;
		if (majorVersion == 14 && minorVersion == 10)
		{
			studioVersion = "2017";
			linkerVersion = "14.1";
		}
		else if (majorVersion == 14 && minorVersion == 0)
		{
			studioVersion = "2015";
			linkerVersion = "14.0";
		}
		else if (majorVersion == 12 && minorVersion == 0)
		{
			studioVersion = "2013";
			linkerVersion = "12.0";
		}
		else if (majorVersion == 11 && minorVersion == 0)
		{
			studioVersion = "2012";
			linkerVersion = "11.0";
		}
		else if (majorVersion == 10 && minorVersion == 0)
		{
			studioVersion = "2010";
			linkerVersion = "10.0";
		}
		else if (majorVersion == 9 && minorVersion == 0)
		{
			studioVersion = "2008";
			linkerVersion = "9.0";
		}
		else if (majorVersion == 8 && minorVersion == 0)
		{
			studioVersion = "2005";
			linkerVersion = "8.0";
		}
		else if (majorVersion == 7 && minorVersion == 10)
		{
			studioVersion = ".NET 2003";
			linkerVersion = "7.1";
		}
		else if (majorVersion == 7 && minorVersion == 0)
		{
			studioVersion = ".NET 2002";
			linkerVersion = "7.0";
		}
		else if (majorVersion == 6 && minorVersion == 0)
		{
			studioVersion = "6.0";
			linkerVersion = "6.0";
		}
		else if (majorVersion == 5 && minorVersion == 0)
		{
			studioVersion = "97";
			linkerVersion = "5.0";
		}
		else if (majorVersion == 4 && minorVersion == 0)
		{
			linkerVersion = "4.x";
		}

		if (linkerVersion.empty())
		{
			// Version did not match with any known version
			return;
		}

		// Add linker
		addLinker(source, strength, "Microsoft Linker", linkerVersion);

		// Add more info if we are sure that binary is MSVC
		if (isDetected("MSVC", DetectResult::DetectionStrength::MEDIUM))
		{
			// MSVC version is same as linker except for v14.1
			auto msvcVersion = linkerVersion == "14.1" ? "15.0" : linkerVersion;

			// Detect possible debug version
			for (const auto tool : toolInfo.detectedTools)
			{
				// Check for debug keyword in signature detections
				if (tool.isCompiler() && tool.name == "MSVC"
					&& contains(tool.versionInfo, "debug"))
				{
					msvcVersion += " debug";
					break;
				}
			}

			studioVersion = "Visual Studio " + studioVersion;
			addCompiler(source, strength, "MSVC", msvcVersion, studioVersion);

			// Do not add language if MSIL is detected
			if (!pe_format->isDotNet() && !pe_format->isPackedDotNet())
			{
				addLanguage("C++");
			}
		}
	}

	void PeHeuristics::getRdataHeuristic()
	{
		auto source = DetectResult::DetectionMethod::COMMENT_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		std::string content;
		const auto section = pe_format->getPeSection(".rdata");
		if (!section || !section->getString(content))
		{
			return;
		}

		std::vector<std::string> records;
		separateStrings(content, records);

		std::string name, version, extra;
		for (const auto& record : records)
		{
			// AutoIt detections
			if (startsWith(record, "This is a third-party compiled AutoIt script.")
				|| startsWith(record, "This is a compiled AutoIt script."))
			{
				name = "Aut2Exe";
				version = "3.x";
				addPriorityLanguage("AutoIt", "", true);
				break;
			}
			else if (startsWith(record, "Compiled AutoIt"))
			{
				name = "Aut2Exe";
				version = "2.x";
				addPriorityLanguage("AutoIt", "", true);
				break;
			}
		}

		if (!name.empty())
		{
			addCompiler(source, strength, name, version, extra);
		}
	}

	void PeHeuristics::getNullsoftHeuristic()
	{
		auto source = DetectResult::DetectionMethod::SECTION_TABLE_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		const auto section = pe_format->getPeSection(".ndata");
		if (section && section->getAddress() && !section->getOffset())
		{
			unsigned long long address;
			if (section->getSizeInMemory(address) && address)
			{
				addInstaller(source, strength, "Nullsoft Install System");
			}
		}
	}

	std::string PeHeuristics::getNullsoftManifestVersion(const std::string& manifest)
	{
		const auto offset = manifest.find("Nullsoft Install System");
		if (offset != std::string::npos)
		{
			const auto verStart = manifest.find('v', offset + 23);
			const auto verEnd = manifest.find('<', offset + 23);

			std::string version;
			if (verEnd < manifest.size())
			{
				version = manifest.substr(verStart + 1, verEnd - verStart - 1);
				if (regex_match(version, std::regex("[[:digit:]]+.[[:digit:]]+")))
				{
					return version;
				}
			}
		}

		return std::string();
	}

	void PeHeuristics::getManifestHeuristic()
	{
		auto source = DetectResult::DetectionMethod::COMBINED;
		auto strength = DetectResult::DetectionStrength::HIGH;

		std::string manifest;
		const auto* resource = pe_format->getManifestResource();
		if (!resource || !resource->getString(manifest))
		{
			return;
		}

		tinyxml2::XMLDocument parsedManifest;
		if (parsedManifest.Parse(
			manifest.c_str(),
			manifest.length()) != tinyxml2::XML_SUCCESS)
		{
			return;
		}

		if (isDetected("Nullsoft"))
		{
			auto version = getNullsoftManifestVersion(manifest);
			if (!version.empty())
			{
				addInstaller(source, strength, "Nullsoft Install System", version);
				return;
			}
		}

		if (isDetected("WinZip SFX"))
		{
			if (auto* root = parsedManifest.FirstChildElement("assembly"))
			{
				if (auto* identity = root->FirstChildElement("assemblyIdentity"))
				{
					if (endsWith(identity->Attribute("name"), "WZSFX")
						|| endsWith(identity->Attribute("name"), "WZSEPE32"))
					{
						std::string version = identity->Attribute("version");
						addInstaller(
							source,
							strength,
							"WinZip SFX",
							version.substr(0, 3)
						);
						return;
					}
				}
			}
		}

		if (pe_format->getOverlaySize() && contains(manifest, "WinRAR SFX module"))
		{
			std::string magic;
			if (pe_format->getStringBytes(magic, declaredLength, 4))
			{
				if (magic == "Rar!")
				{
					addInstaller(source, strength, "WinRAR SFX");
					return;
				}

				if (startsWith(magic, "PK"))
				{
					addInstaller(
						source,
						strength,
						"WinRAR SFX",
						"",
						"with ZIP payload"
					);
					return;
				}
			}
		}

		if (pe_format->getOverlaySize() && contains(manifest, "WinRAR archiver"))
		{
			std::string magic;
			if (pe_format->getStringBytes(magic, declaredLength, 4))
			{
				if (magic == "Rar!")
				{
					addInstaller(
						source,
						strength,
						"WinRAR SFX",
						"",
						"console version"
					);
					return;
				}
			}
		}
	}

	void PeHeuristics::getSevenZipHeuristics()
	{
		auto source = DetectResult::DetectionMethod::COMBINED;
		auto strength = DetectResult::DetectionStrength::HIGH;

		if (!pe_format->getOverlaySize())
		{
			return;
		}

		bool detected = false;
		std::string magic;
		if (pe_format->getStringBytes(magic, declaredLength, 18))
		{
			if (magic == ";!@Install@!UTF-8!")
			{
				detected = true;
			}
		}
		if (pe_format->getStringBytes(magic, declaredLength, 6))
		{
			if (magic == "7z\xBC\xAF\x27\x1C")
			{
				detected = true;
			}
		}

		if (detected)
		{
			auto& resourceTable = pe_format->getResourceTable();
			if (resourceTable)
			{
				// See: VS_VERSIONINFO structure documentation
				auto resource = resourceTable->getResourceWithType(16);
				if (resource && resource->isLoaded())
				{
					std::uint64_t infoL = 0;
					auto offset = resource->getOffset();
					pe_format->getXByteOffset(offset + 2, 2, infoL, PeFormat::Endianness::LITTLE);

					if (infoL)
					{
						offset += 0x38; // skip to product version - minor
						std::uint64_t minV = 0;
						pe_format->getXByteOffset(offset, 2, minV, PeFormat::Endianness::LITTLE);
						offset += 0x02; // skip to product version - major
						std::uint64_t majV = 0;
						pe_format->getXByteOffset(offset, 2, majV, PeFormat::Endianness::LITTLE);

						std::stringstream version;
						version << majV << "." << std::setfill('0') << std::setw(2)
							<< minV;
						addInstaller(source, strength, "7-Zip SFX", version.str());
					}
				} // resource
			} // resource table
		}
	}

	void PeHeuristics::getMewSectionHeuristics()
	{
		auto source = DetectResult::DetectionMethod::SECTION_TABLE_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		std::string version;
		if (noOfSections == 2)
		{
			if (startsWith(sections[0]->getName(), "MEWF"))
			{
				version = "11 SE 1.x";
			}
			else if (sections[0]->getName() == ".data"
				&& sections[1]->getName() == ".decode")
			{
				version = "11 SE 1.x";
			}
		}

		if (!version.empty())
		{
			addPacker(source, strength, "MEW", version);
		}
	}

	void PeHeuristics::getNsPackSectionHeuristics()
	{
		auto source = DetectResult::DetectionMethod::SECTION_TABLE_H;
		auto strength = DetectResult::DetectionStrength::MEDIUM;

		if (!noOfSections)
		{
			return;
		}

		const auto& firstName = sections[0]->getName();
		if (firstName == "nsp0" || firstName == ".nsp0")
		{
			const auto namePrefix = firstName.substr(0, firstName.length() - 1);

			std::size_t counter = 0;
			for (std::size_t i = 1; i < noOfSections; ++i)
			{
				if (sections[i]->getName() != (namePrefix + std::to_string(i)))
				{
					if (++counter > 1)
					{
						return;
					}
				}
			}

			auto version = sections[0]->getName() == "nsp0" ? "2.x" : "3.x";
			addPacker(source, strength, "NsPack", version);
		}
	}

	void PeHeuristics::getPeSectionHeuristics()
	{
		auto source = DetectResult::DetectionMethod::SECTION_TABLE_H;
		auto strength = DetectResult::DetectionStrength::HIGH;

		if (!noOfSections)
		{
			return;
		}

		// Get often used names
		const auto firstName = sections[0]->getName();
		const auto lastName = sections[noOfSections - 1]->getName();

		// Get often used conditional names
		const auto secondName = noOfSections > 1 ? sections[1]->getName() : "";
		const auto secondLastName = noOfSections > 2
			? sections[noOfSections - 2]->getName()
			: "";
		const auto epName = toolInfo.entryPointSection
			? toolInfo.epSection.getName()
			: "";

		// Installer detections
		if (lastName == "_winzip_")
		{
			addInstaller(source, strength, "WinZip SFX");
		}

		// Other tools
		if (findSectionName(".mackt") >= 1)
		{
			toolInfo.addTool(
				source,
				strength,
				DetectResult::ToolType::OTHER,
				"ImpREC reconstructed"
			);
		}
		if (findSectionName(".winapi") >= 1)
		{
			toolInfo.addTool(
				source,
				strength,
				DetectResult::ToolType::OTHER,
				"API Override tool"
			);
		}

		// Packer detections
		if (lastName == ".taz")
		{
			addPacker(source, strength, "PESpin");
		}
		if (lastName == ".ccg")
		{
			addPacker(source, strength, "CCG packer");
		}
		if (lastName == ".svkp")
		{
			addPacker(source, strength, "SVKProtector");
		}
		if (lastName == "PEPACK!!")
		{
			addPacker(source, strength, "PE-PACK");
		}
		if (lastName == ".WWP32")
		{
			addPacker(source, strength, "WWPack32");
		}
		if (lastName == "lamecryp")
		{
			addPacker(source, strength, "LameCrypt");
		}
		if (firstName == ".Upack" || firstName == ".ByDwing")
		{
			addPacker(source, strength, "Upack");
		}
		if (lastName == "yC" || lastName == ".y0da" || lastName == ".yP")
		{
			addPacker(source, strength, "yoda's Protector");
		}
		if (findSectionName(".petite") == 1)
		{
			addPacker(source, strength, "Petite");
		}
		if (findSectionName(".pklstb") == 1)
		{
			addPacker(source, strength, "PKLite");
		}
		if (findSectionName("hmimys") == 1)
		{
			addPacker(source, strength, "hmimys");
		}
		if (findSectionName(".securom") == 1)
		{
			addPacker(source, strength, "SecuROM");
		}
		if (findSectionName(".neolit") == 1 || findSectionName(".neolite") == 1)
		{
			addPacker(source, strength, "NeoLite");
		}
		if (findSectionName("RCryptor") == 1 || findSectionName(".RCrypt") == 1)
		{
			addPacker(source, strength, "RCryptor");
		}
		if (findSectionName(".MPRESS1") == 1 && findSectionName(".MPRESS2") == 1)
		{
			addPacker(source, strength, "MPRESS");
		}
		if (findSectionName(".dyamarC") == 1 && findSectionName(".dyamarD") == 1)
		{
			addPacker(source, strength, "DYAMAR");
		}
		if (findSectionName("krypton") == 1 && findSectionName("YADO") >= 1)
		{
			addPacker(source, strength, "Krypton");
		}
		if (findSectionName(".boom") >= 1)
		{
			addPacker(source, strength, "The Boomerang");
		}
		if (findSectionName("DAStub") >= 1)
		{
			addPacker(source, strength, "DAStub Dragon Armor Protector");
		}
		if (findSectionName("!EPack") >= 1)
		{
			addPacker(source, strength, "EPack");
		}
		if (findSectionName(".MaskPE") >= 1)
		{
			addPacker(source, strength, "MaskPE");
		}
		if (findSectionName(".perplex") >= 1)
		{
			addPacker(source, strength, "Perplex PE Protector");
		}
		if (findSectionName("ProCrypt") >= 1)
		{
			addPacker(source, strength, "ProCrypt");
		}
		if (findSectionName(".seau") >= 1)
		{
			addPacker(source, strength, "SeauSFX");
		}
		if (findSectionName(".spack") >= 1)
		{
			addPacker(source, strength, "Simple Pack");
		}
		if (findSectionName(".charmve") >= 1 || findSectionName(".pinclie") >= 1)
		{
			addPacker(source, strength, "PIN tool");
		}
		if (epName == "TheHyper")
		{
			addPacker(source, strength, "TheHyper's protector");
		}
		if (startsWith(epName, "Themida"))
		{
			addPacker(source, strength, "Themida");
		}
		if (findSectionName("NFO") == noOfSections)
		{
			addPacker(source, strength, "NFO");
		}
		if (findSectionName("kkrunchy") == 1 && noOfSections == 1)
		{
			addPacker(source, strength, "kkrunchy");
		}
		if (noOfSections > 1)
		{
			if (lastName == "pebundle" && secondLastName == "pebundle")
			{
				addPacker(source, strength, "PEBundle");
			}
		}
		if (noOfSections == 2)
		{
			if (firstName == ".packed" && lastName == ".RLPack")
			{
				addPacker(source, strength, "RLPack");
			}
			if (firstName == ".rsrc" && lastName == "coderpub")
			{
				addPacker(source, strength, "DxPack");
			}
		}
		if (noOfSections > 2)
		{
			if (firstName == "UPX0" && secondName == "UPX1"
				&& sections[0]->getSizeInFile() == 0)
			{
				addPacker(source, strength, "UPX");
			}
			if (lastName == ".data" && secondLastName == ".data"
				&& findSectionName("") == noOfSections - 2)
			{
				addPacker(source, strength, "ASProtect");
			}
		}
		if (noOfSections >= 2)
		{
			if (findSectionName("BitArts") == noOfSections - 1)
			{
				addPacker(source, strength, "Crunch/PE");
			}
			if (secondLastName == ".tsustub" && lastName == ".tsuarch")
			{
				addPacker(source, strength, "TSULoader");
			}
			if (secondLastName == ".gentee")
			{
				addCompiler(source, strength, "Gentee");
			}
		}
		if (firstName == "pec1"
			&& epName == "pec2"
			&& toolInfo.epSection.getIndex() == 1)
		{
			addPacker(source, strength, "PECompact", "1.xx");
		}
		if (epName == "ExeS" && toolInfo.epSection.getSizeInFile() == 0xD9F
			&& startsWith(toolInfo.epBytes, "EB00EB"))
		{
			addPacker(source, strength, "EXE Stealth", "2.72 - 2.73");
		}
		if (epName == ".aspack")
		{
			auto epSecIndex = toolInfo.epSection.getIndex();
			if (epSecIndex + 1 < noOfSections - 1
				&& sections[epSecIndex + 1]->getName() == ".adata")
			{
				addPacker(source, strength, "ASPack");
			}
		}

		std::size_t sameName = 0;
		if ((sameName = findSectionName(".pelock")) && sameName >= noOfSections - 1)
		{
			addPacker(source, strength, "PELock", "1.x");
		}
		if ((sameName = findSectionName("PELOCKnt"))
			&& (sameName >= noOfSections - 2 || noOfSections < 2))
		{
			addPacker(source, strength, "PELock", "NT");
		}

		getMewSectionHeuristics();
		getNsPackSectionHeuristics();
	}

	void PeHeuristics::getFormatSpecificLanguageHeuristics()
	{
		getGoHeuristics();
		getAutoItHeuristics();
		getDotNetHeuristics();
		getVisualBasicHeuristics();
	}

	void PeHeuristics::getFormatSpecificCompilerHeuristics()
	{
		getHeaderStyleHeuristics();
		getSlashedSignatures();
		getMorphineHeuristics();
		getStarForceHeuristics();
		getSafeDiscHeuristics();
		getSecuROMHeuristics();
		getMPRMMGVAHeuristics();
		getActiveMarkHeuristics();
		getRLPackHeuristics();
		getPetiteHeuristics();
		getPelockHeuristics();
		getEzirizReactorHeuristics();
		getUpxHeuristics();
		getFsgHeuristics();
		getPeCompactHeuristics();
		getAndpakkHeuristics();
		getEnigmaHeuristics();
		getVBoxHeuristics();
		getActiveDeliveryHeuristics();
		getAdeptProtectorHeuristics();
		getCodeLockHeuristics();
		getNetHeuristic();
		getExcelsiorHeuristics();
		getVmProtectHeuristics();
		getBorlandDelphiHeuristics();
		getBeRoHeuristics();
		getMsvcIntelHeuristics();
		getStarforceHeuristic();
		getArmadilloHeuristic();
		getRdataHeuristic();
		getNullsoftHeuristic();
		getLinkerVersionHeuristic();
		getManifestHeuristic();
		getSevenZipHeuristics();
		getPeSectionHeuristics();
	}
} // namespace cchips
