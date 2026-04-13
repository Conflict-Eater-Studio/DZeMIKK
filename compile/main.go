package main

import (
	"bytes"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"sync"

	"github.com/charmbracelet/bubbles/filepicker"
	tea "github.com/charmbracelet/bubbletea"
	"github.com/charmbracelet/lipgloss"
)

type stage int

const (
	stageSelectPreset stage = iota
	stageSelectGenerator
	stagePickFMOD
	stageRunning
	stageDone
)

type buildPreset struct {
	label      string
	buildType  string
	binaryDir  string
	successMsg string
}

type buildResultMsg struct {
	output     string
	err        error
	outputFile string
	writeErr   error
}

type buildStreamMsg struct {
	chunk string
}

type model struct {
	stage stage

	presetCursor    int
	generatorCursor int
	presets         []buildPreset
	generators      []string
	selectedPreset  buildPreset
	selectedGen     string

	filePicker filepicker.Model
	fmodPath   string
	envHasFMOD bool

	rootDir    string
	execDir    string
	outputFile string

	status   string
	output   string
	err      error
	writeErr error

	logCh  <-chan string
	doneCh <-chan buildResultMsg
}

func initialModel() (model, error) {
	execDir, err := os.Getwd()
	if err != nil {
		return model{}, err
	}

	rootDir, err := findProjectRoot()
	if err != nil {
		return model{}, err
	}

	envFMOD := strings.TrimSpace(os.Getenv("FMOD_LIBS_PATH"))
	hasEnvFMOD := envFMOD != ""

	homeDir, homeErr := os.UserHomeDir()
	if homeErr != nil {
		homeDir = execDir
	}

	fp := filepicker.New()
	fp.CurrentDirectory = homeDir
	fp.ShowHidden = false
	fp.ShowPermissions = false
	fp.Height = 14
	fp.DirAllowed = true
	fp.FileAllowed = false

	m := model{
		stage:           stageSelectPreset,
		presetCursor:    0,
		generatorCursor: 0,
		presets: []buildPreset{
			{label: "Debug", buildType: "Debug", binaryDir: "cmake-build-debug", successMsg: "Debug build finished successfully."},
			{label: "Release", buildType: "Release", binaryDir: "cmake-build-release", successMsg: "Release build finished successfully."},
			{label: "RelWithDebInfo", buildType: "RelWithDebInfo", binaryDir: "cmake-build-relwithdebinfo", successMsg: "RelWithDebInfo build finished successfully."},
		},
		generators:  []string{"Ninja", "MSVC (Visual Studio 17 2022)"},
		selectedGen: "Ninja",
		filePicker:  fp,
		fmodPath:    envFMOD,
		envHasFMOD:  hasEnvFMOD,
		rootDir:     rootDir,
		execDir:     execDir,
		outputFile:  filepath.Join(execDir, "cmake_output.txt"),
		status:      "Select a build preset.",
	}
	m.selectedPreset = m.presets[0]

	if hasEnvFMOD {
		m.status = "FMOD_LIBS_PATH found in environment."
	}

	return m, nil
}

func (m model) Init() tea.Cmd {
	return nil
}

func (m model) Update(msg tea.Msg) (tea.Model, tea.Cmd) {
	switch msg := msg.(type) {
	case tea.KeyMsg:
		switch m.stage {
		case stageSelectPreset:
			return updateSelectPreset(m, msg)
		case stageSelectGenerator:
			return updateSelectGenerator(m, msg)
		case stagePickFMOD:
			return updatePickFMOD(m, msg)
		case stageRunning:
			if msg.String() == "ctrl+c" {
				return m, tea.Quit
			}
			return m, nil
		case stageDone:
			switch msg.String() {
			case "q", "esc", "ctrl+c", "enter":
				return m, tea.Quit
			}
		}

	case buildResultMsg:
		m.output = msg.output
		m.err = msg.err
		m.writeErr = msg.writeErr
		m.outputFile = msg.outputFile
		m.stage = stageDone
		if msg.err != nil {
			m.status = fmt.Sprintf("Build failed: %v", msg.err)
		} else {
			m.status = m.selectedPreset.successMsg
		}
		return m, nil

	case buildStreamMsg:
		m.output += msg.chunk
		if m.logCh != nil || m.doneCh != nil {
			return m, waitForBuildEvent(m.logCh, m.doneCh)
		}
		return m, nil
	}

	if m.stage == stagePickFMOD {
		var cmd tea.Cmd
		m.filePicker, cmd = m.filePicker.Update(msg)
		return m, cmd
	}

	return m, nil
}

func updateSelectPreset(m model, msg tea.KeyMsg) (tea.Model, tea.Cmd) {
	switch msg.String() {
	case "up", "k":
		if m.presetCursor > 0 {
			m.presetCursor--
		}
	case "down", "j":
		if m.presetCursor < len(m.presets)-1 {
			m.presetCursor++
		}
	case "ctrl+c", "q", "esc":
		return m, tea.Quit
	case "enter":
		m.selectedPreset = m.presets[m.presetCursor]
		m.stage = stageSelectGenerator
		m.status = "Select a CMake generator."
	}
	return m, nil
}

func updateSelectGenerator(m model, msg tea.KeyMsg) (tea.Model, tea.Cmd) {
	switch msg.String() {
	case "up", "k":
		if m.generatorCursor > 0 {
			m.generatorCursor--
		}
	case "down", "j":
		if m.generatorCursor < len(m.generators)-1 {
			m.generatorCursor++
		}
	case "ctrl+c", "q", "esc":
		return m, tea.Quit
	case "enter":
		m.selectedGen = m.generators[m.generatorCursor]
		if m.envHasFMOD {
			return startBuild(m)
		}

		m.stage = stagePickFMOD
		m.status = "FMOD_LIBS_PATH was not found. Select a folder."
		return m, nil
	}
	return m, nil
}

func updatePickFMOD(m model, msg tea.KeyMsg) (tea.Model, tea.Cmd) {
	switch msg.String() {
	case "ctrl+c", "q", "esc":
		return m, tea.Quit
	case "ctrl+s":
		selectedDir := strings.TrimSpace(m.filePicker.CurrentDirectory)
		if selectedDir == "" {
			m.status = "No directory selected yet."
			return m, nil
		}
		if stat, err := os.Stat(selectedDir); err != nil || !stat.IsDir() {
			m.status = "Selected path is not a valid directory."
			return m, nil
		}

		m.fmodPath = selectedDir
		return startBuild(m)
	}

	var cmd tea.Cmd
	m.filePicker, cmd = m.filePicker.Update(msg)

	if didSelect, path := m.filePicker.DidSelectFile(msg); didSelect {
		selectedDir := strings.TrimSpace(path)
		if stat, err := os.Stat(selectedDir); err == nil && stat.IsDir() {
			m.fmodPath = selectedDir
			return startBuild(m)
		}
	}

	return m, cmd
}

func (m model) cmakeGenerator() string {
	if strings.HasPrefix(m.selectedGen, "MSVC") {
		return "Visual Studio 17 2022"
	}
	return "Ninja"
}

func (m model) View() string {
	var b strings.Builder

	titleStyle := lipgloss.NewStyle().Bold(true).Foreground(lipgloss.Color("39"))
	headerStyle := lipgloss.NewStyle().Foreground(lipgloss.Color("214")).Bold(true)
	okStyle := lipgloss.NewStyle().Foreground(lipgloss.Color("42")).Bold(true)
	errStyle := lipgloss.NewStyle().Foreground(lipgloss.Color("160")).Bold(true)
	mutedStyle := lipgloss.NewStyle().Foreground(lipgloss.Color("245"))
	panelStyle := lipgloss.NewStyle().Border(lipgloss.NormalBorder()).BorderForeground(lipgloss.Color("63")).Padding(1, 2)

	b.WriteString(titleStyle.Render("DZeMIKK CMake Builder"))
	b.WriteString("\n\n")
	b.WriteString(mutedStyle.Render(fmt.Sprintf("Project root: %s", m.rootDir)))
	b.WriteString("\n")
	b.WriteString(mutedStyle.Render(fmt.Sprintf("Output log: %s", m.outputFile)))
	b.WriteString("\n")
	b.WriteString(headerStyle.Render(fmt.Sprintf("Status: %s", m.status)))
	b.WriteString("\n\n")

	switch m.stage {
	case stageSelectPreset:
		b.WriteString(headerStyle.Render("Select build preset"))
		b.WriteString("\n")
		for i, p := range m.presets {
			cursor := " "
			if m.presetCursor == i {
				cursor = ">"
			}
			b.WriteString(fmt.Sprintf(" %s %s\n", cursor, p.label))
		}
		b.WriteString("\n")
		b.WriteString(mutedStyle.Render("Controls: up/down, enter, q"))

	case stageSelectGenerator:
		b.WriteString(headerStyle.Render("Select CMake generator"))
		b.WriteString("\n")
		for i, g := range m.generators {
			cursor := " "
			if m.generatorCursor == i {
				cursor = ">"
			}
			b.WriteString(fmt.Sprintf(" %s %s\n", cursor, g))
		}
		if m.envHasFMOD {
			b.WriteString("\n")
			b.WriteString(okStyle.Render(fmt.Sprintf("Detected FMOD_LIBS_PATH from env: %s", m.fmodPath)))
			b.WriteString("\n")
		}
		b.WriteString("\n")
		b.WriteString(mutedStyle.Render("Controls: up/down, enter, q"))

	case stagePickFMOD:
		b.WriteString(headerStyle.Render("Select FMOD_LIBS_PATH folder"))
		b.WriteString("\n")
		b.WriteString(m.filePicker.View())
		b.WriteString("\n")
		b.WriteString(mutedStyle.Render("Controls: arrows to move, right to open folder, enter to select folder, left/backspace to go back, Ctrl+S to select current directory, q to quit"))
		b.WriteString("\n")
		b.WriteString(mutedStyle.Render(fmt.Sprintf("Current directory: %s", m.filePicker.CurrentDirectory)))

	case stageRunning:
		b.WriteString(headerStyle.Render("Running build, please wait..."))
		b.WriteString("\n")
		b.WriteString(mutedStyle.Render("Press Ctrl+C to quit UI."))
		b.WriteString("\n\n")
		runningOutput := tailLines(m.output, 40)
		if strings.TrimSpace(runningOutput) == "" {
			b.WriteString(mutedStyle.Render("Waiting for build output..."))
		} else {
			b.WriteString(panelStyle.Render(runningOutput))
		}

	case stageDone:
		if m.output != "" {
			outputPanel := panelStyle.Render(m.output)
			b.WriteString(outputPanel)
			b.WriteString("\n")
		}
		if m.err != nil {
			b.WriteString(errStyle.Render("Build failed."))
			b.WriteString("\n")
		} else {
			b.WriteString(okStyle.Render("Build completed successfully."))
			b.WriteString("\n")
		}
		if m.writeErr != nil {
			b.WriteString(errStyle.Render(fmt.Sprintf("Could not write cmake_output.txt: %v", m.writeErr)))
			b.WriteString("\n")
		} else {
			b.WriteString(okStyle.Render(fmt.Sprintf("Saved output to: %s", m.outputFile)))
			b.WriteString("\n")
		}
		b.WriteString(mutedStyle.Render("Press Enter or q to quit."))
	}

	return lipgloss.NewStyle().Padding(1, 2).Render(b.String())
}

func startBuild(m model) (tea.Model, tea.Cmd) {
	m.stage = stageRunning
	m.status = "Running CMake configure + build..."
	m.output = ""

	logCh := make(chan string, 256)
	doneCh := make(chan buildResultMsg, 1)
	m.logCh = logCh
	m.doneCh = doneCh

	rootDir := m.rootDir
	outputFile := m.outputFile
	preset := m.selectedPreset
	generator := m.cmakeGenerator()
	fmodPath := m.fmodPath

	go func() {
		output, err := doBuild(rootDir, preset, generator, fmodPath, func(chunk string) {
			logCh <- chunk
		})
		writeErr := os.WriteFile(outputFile, []byte(output), 0644)
		close(logCh)
		doneCh <- buildResultMsg{output: output, err: err, outputFile: outputFile, writeErr: writeErr}
	}()

	return m, waitForBuildEvent(logCh, doneCh)
}

func waitForBuildEvent(logCh <-chan string, doneCh <-chan buildResultMsg) tea.Cmd {
	return func() tea.Msg {
		for {
			select {
			case chunk, ok := <-logCh:
				if ok {
					return buildStreamMsg{chunk: chunk}
				}
				logCh = nil
			case result := <-doneCh:
				return result
			}
		}
	}
}

func doBuild(rootDir string, preset buildPreset, generator, fmodPath string, emit func(string)) (string, error) {
	var out bytes.Buffer

	cfgArgs := []string{
		"-S", ".",
		"-B", preset.binaryDir,
		fmt.Sprintf("-DCMAKE_BUILD_TYPE=%s", preset.buildType),
		fmt.Sprintf("-DFMOD_LIBS_PATH=%s", fmodPath),
		"-G", generator,
	}
	if err := runCmd(rootDir, &out, emit, "cmake", cfgArgs...); err != nil {
		return out.String(), fmt.Errorf("configure step failed: %w", err)
	}

	buildArgs := []string{"--build", preset.binaryDir, "--target", "game", "-j", "10"}
	if err := runCmd(rootDir, &out, emit, "cmake", buildArgs...); err != nil {
		return out.String(), fmt.Errorf("build step failed: %w", err)
	}

	out.WriteString("\n")
	out.WriteString(preset.successMsg)
	out.WriteString("\n")
	return out.String(), nil
}

type streamWriter struct {
	mu   sync.Mutex
	out  *bytes.Buffer
	emit func(string)
}

func (w *streamWriter) Write(p []byte) (int, error) {
	w.mu.Lock()
	defer w.mu.Unlock()

	n, err := w.out.Write(p)
	if n > 0 && w.emit != nil {
		w.emit(string(p[:n]))
	}
	return n, err
}

func runCmd(dir string, out *bytes.Buffer, emit func(string), name string, args ...string) error {
	commandLine := fmt.Sprintf("$ %s %s\n", name, strings.Join(args, " "))
	out.WriteString(commandLine)
	if emit != nil {
		emit(commandLine)
	}

	cmd := exec.Command(name, args...)
	cmd.Dir = dir
	w := &streamWriter{out: out, emit: emit}
	cmd.Stdout = w
	cmd.Stderr = w
	return cmd.Run()
}

func tailLines(input string, maxLines int) string {
	if maxLines <= 0 {
		return ""
	}

	lines := strings.Split(input, "\n")
	if len(lines) <= maxLines {
		return input
	}

	start := len(lines) - maxLines
	return strings.Join(lines[start:], "\n")
}

func findProjectRoot() (string, error) {
	cwd, err := os.Getwd()
	if err != nil {
		return "", err
	}

	for {
		candidate := filepath.Join(cwd, "CMakeLists.txt")
		if _, err := os.Stat(candidate); err == nil {
			return cwd, nil
		}

		parent := filepath.Dir(cwd)
		if parent == cwd {
			break
		}
		cwd = parent
	}

	return "", errors.New("could not find project root containing CMakeLists.txt")
}

func main() {
	m, err := initialModel()
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error: %v\n", err)
		os.Exit(1)
	}

	p := tea.NewProgram(m)
	if _, err := p.Run(); err != nil {
		fmt.Fprintf(os.Stderr, "Program error: %v\n", err)
		os.Exit(1)
	}
}
