#!/bin/bash
# Generate changelog from git history
# Usage: ./generate-changelog.sh [VERSION] [SINCE_TAG]

set -e

VERSION="${1:-HEAD}"
SINCE_TAG="${2:-}"
OUTPUT_FILE="${3:-CHANGELOG.md}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Generating changelog for version ${VERSION}...${NC}"

# Get the date
DATE=$(date +"%Y-%m-%d")

# Start changelog
cat > "${OUTPUT_FILE}" << EOF
# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [${VERSION}] - ${DATE}

EOF

# Determine the range of commits to include
if [ -z "${SINCE_TAG}" ]; then
    # Find the last tag
    SINCE_TAG=$(git describe --tags --abbrev=0 2>/dev/null || echo "")
fi

if [ -z "${SINCE_TAG}" ]; then
    echo -e "${YELLOW}No previous tags found. Including all commits.${NC}"
    RANGE=""
else
    echo -e "${GREEN}Generating changelog since ${SINCE_TAG}${NC}"
    RANGE="${SINCE_TAG}..HEAD"
fi

# Function to format commit message
format_commit() {
    local commit=$1
    local hash=$(echo "$commit" | cut -d' ' -f1)
    local message=$(echo "$commit" | cut -d' ' -f2-)
    
    # Remove common prefixes and format
    message=$(echo "$message" | sed -E 's/^(feat|fix|docs|style|refactor|test|chore)(\(.*\))?:\s*//')
    
    echo "- ${message} ([${hash}](https://github.com/blazeiburgess/WaoN/commit/${hash}))"
}

# Categorize commits
echo "### Added" >> "${OUTPUT_FILE}"
git log ${RANGE} --pretty=format:"%h %s" --grep="^feat" --grep="^feature" | while read commit; do
    [ -n "$commit" ] && format_commit "$commit" >> "${OUTPUT_FILE}"
done || echo "- No new features" >> "${OUTPUT_FILE}"

echo -e "\n### Changed" >> "${OUTPUT_FILE}"
git log ${RANGE} --pretty=format:"%h %s" --grep="^refactor" --grep="^perf" | while read commit; do
    [ -n "$commit" ] && format_commit "$commit" >> "${OUTPUT_FILE}"
done || echo "- No changes" >> "${OUTPUT_FILE}"

echo -e "\n### Fixed" >> "${OUTPUT_FILE}"
git log ${RANGE} --pretty=format:"%h %s" --grep="^fix" --grep="^bugfix" | while read commit; do
    [ -n "$commit" ] && format_commit "$commit" >> "${OUTPUT_FILE}"
done || echo "- No fixes" >> "${OUTPUT_FILE}"

echo -e "\n### Documentation" >> "${OUTPUT_FILE}"
git log ${RANGE} --pretty=format:"%h %s" --grep="^docs" | while read commit; do
    [ -n "$commit" ] && format_commit "$commit" >> "${OUTPUT_FILE}"
done || echo "- No documentation updates" >> "${OUTPUT_FILE}"

# Add remaining commits
echo -e "\n### Other" >> "${OUTPUT_FILE}"
git log ${RANGE} --pretty=format:"%h %s" | grep -v -E "^[a-f0-9]+ (feat|feature|fix|bugfix|docs|style|refactor|test|chore)" | while read commit; do
    [ -n "$commit" ] && format_commit "$commit" >> "${OUTPUT_FILE}"
done || echo "- No other changes" >> "${OUTPUT_FILE}"

# Add previous versions if generating full changelog
if [ "${OUTPUT_FILE}" = "CHANGELOG.md" ] && [ -n "${SINCE_TAG}" ]; then
    echo -e "\n---\n" >> "${OUTPUT_FILE}"
    
    # Get all tags in reverse chronological order
    git tag -l --sort=-version:refname | while read tag; do
        if [ "${tag}" != "${VERSION}" ]; then
            TAG_DATE=$(git log -1 --format=%ai "${tag}" | cut -d' ' -f1)
            echo -e "\n## [${tag}] - ${TAG_DATE}" >> "${OUTPUT_FILE}"
            
            # Get previous tag
            PREV_TAG=$(git tag -l --sort=-version:refname | grep -A1 "^${tag}$" | tail -1)
            if [ "${PREV_TAG}" = "${tag}" ]; then
                PREV_TAG=""
            fi
            
            # Generate summary for this version
            if [ -n "${PREV_TAG}" ]; then
                git log "${PREV_TAG}..${tag}" --pretty=format:"- %s" >> "${OUTPUT_FILE}"
            else
                git log "${tag}" --pretty=format:"- %s" >> "${OUTPUT_FILE}"
            fi
        fi
    done
fi

# Add links section
echo -e "\n\n[${VERSION}]: https://github.com/blazeiburgess/WaoN/releases/tag/v${VERSION}" >> "${OUTPUT_FILE}"

echo -e "${GREEN}Changelog generated successfully: ${OUTPUT_FILE}${NC}"

# Optional: Show the changelog
if [ -t 1 ]; then
    echo -e "\n${YELLOW}Preview:${NC}"
    head -n 30 "${OUTPUT_FILE}"
    echo -e "\n..."
fi