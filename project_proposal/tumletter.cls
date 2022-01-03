%
% tumletter.cls
%

%----------------------------------------------------------------------
% Identify class
%
\NeedsTeXFormat{LaTeX2e}[1999/12/01]
\ProvidesClass{tumletter}%
    [2016/09/21 v0.1 %
    tumletter.cls letter class]

%----------------------------------------------------------------------
% Class options
%
\ProcessOptions\relax

\newif\iftumletter@nopagenum
\tumletter@nopagenumfalse

\DeclareOption{nopagenum}{%
    \tumletter@nopagenumtrue%
}
\ProcessOptions\relax


\LoadClassWithOptions{article}
\RequirePackage{tumcolor}
\RequirePackage{tumcontact}
\RequirePackage{tumlang}
\RequirePackage{xparse}
\RequirePackage[pagestyles]{titlesec}
\RequirePackage{geometry}
\RequirePackage{tikz}
\usetikzlibrary{calc}
\RequirePackage{tikz}
\usetikzlibrary{calc}

\newgeometry{%
	top		    = 15mm,
	left		= 2cm,
	right		= 2cm,
	bottom		= 3cm,
	headheight	= 1cm,
	headsep		= 2mm,
	footskip	= 2cm,
    includehead
}

\setlength{\parindent}{0pt}
\setlength{\parskip}{\baselineskip}

\def\tumletter@thename{}
\newcommand{\name}[1]{%
    \def\tumletter@thename{#1}
}

\def\tumletter@salutation{}
\newcommand{\salutation}[1]{%
    \def\tumletter@salutation{#1}
}

\def\tumletter@theanrede{}
\newcommand{\anrede}[1]{%
    \def\tumletter@theanrede{#1}
}
\def\tumletter@thesubject{}
\newcommand{\subject}[1]{%
    \def\tumletter@thesubject{#1}
}

\def\tumletter@thephone{}
\newcommand{\phone}[1]{%
    \def\tumletter@thephone{#1}
}
\def\tumletter@thefax{}
\newcommand{\fax}[1]{%
    \def\tumletter@thefax{#1}
}
\def\tumletter@themail{}
\newcommand{\mail}[1]{%
    \def\tumletter@themail{#1}
}
\def\tumletter@theweb{}
\newcommand{\web}[1]{%
    \edef\tumletter@theweb{#1}
}

\def\tumletter@thestreet{}
\def\tumletter@thestreetno{}
\def\tumletter@thezipcode{}
\def\tumletter@thecity{}
\newcommand{\address}[4]{%
    \def\tumletter@thestreet{#1}%
    \def\tumletter@thestreetno{#2}%
    \def\tumletter@thezipcode{#3}%
    \def\tumletter@thecity{#4}%
}

\def\tumletter@thercptcompany{}
\def\tumletter@thercptname{}
\def\tumletter@thercptstreet{}
\def\tumletter@thercptstreetno{}
\def\tumletter@thercptzipcode{}
\def\tumletter@thercptcity{}
\def\tumletter@thecountry{}
\def\recipient{%
    \@ifnextchar[%
    {\tumletter@recipienti}%
    {\tumletter@recipienti[]}%
}
\def\tumletter@recipienti[#1]#2#3#4#5#6{%
    \@ifnextchar[%
    {\tumletter@recipientii{#1}{#2}{#3}{#4}{#5}{#6}}%
    {\tumletter@recipientii{#1}{#2}{#3}{#4}{#5}{#6}[]}%
}
\def\tumletter@recipientii#1#2#3#4#5#6[#7]{%
    \def\tumletter@thercptcompany{#1}
    \def\tumletter@thercptname{#2}
    \def\tumletter@thercptstreet{#3}
    \def\tumletter@thercptstreetno{#4}
    \def\tumletter@thercptzipcode{#5}
    \def\tumletter@thercptcity{#6}
    \def\tumletter@thecountry{#7}
}

\newcommand{\makerecipient}{%
    \begin{minipage}[t][5mm]{\textwidth}
        \scriptsize%
        TUM I8\ |\ \tumletter@thename\ |\ \tumletter@thestreet\, %
        \tumletter@thestreetno\ |\ \tumletter@thezipcode\ \tumletter@thecity%
    \end{minipage}

    \vspace{-\baselineskip}%
    \vspace{1ex}%
    \begin{minipage}[t][12.7mm]{\textwidth}
    \end{minipage}

    \vspace{-\baselineskip}%
    \vspace{1ex}%
    \begin{minipage}[t][27.3mm]{.5\textwidth}
        \ifx\tumletter@thercptcompany\@empty\relax\else%
            \tumletter@thercptcompany\\%
        \fi%
        \tumletter@thercptname\\%
        \tumletter@thercptstreet\ \tumletter@thercptstreetno\\%
        \tumletter@thercptzipcode\ \tumletter@thercptcity%
        \ifx\tumletter@thecountry\@empty\relax\else%
            \\\tumletter@thecountry%
        \fi%
    \end{minipage}%
    \begin{minipage}[t][27.3mm]{.5\textwidth}%
        \vspace*{\fill}\hfill\today
    \end{minipage}

    \ifx\tumletter@thesubject\@empty\relax\else%
        \vspace{5mm}
        {\bfseries\tumletter@thesubject}
    \fi%

    \vspace{5mm}
    \tumletter@theanrede,\\
}


\newcommand{\makesalutation}{%
    \vspace{1cm}%
    \tumletter@salutation\\%
}

\newcommand{\makesignature}{%
    \vspace*{1cm}%

    \rule{6cm}{.5pt}\\%
    \footnotesize\tumletter@thename%
}

\newcommand{\makefoldmarkings}{%
    \begin{tikzpicture}[overlay,remember picture]
        \draw[black!40,line width=.5pt]
            ($(current page.north west)+(0,-87mm)$) to +(10mm,0);
        \draw[black!40,line width=.5pt]
            ($(current page.south west)+(0,105mm)$) to +(10mm,0);
    \end{tikzpicture}
}


\newpagestyle{tumarticle}{%
	\sethead{}{\TUMheader*{1cm}}{}
    \iftumletter@nopagenum%
	    \setfoot{}{{\scriptsize\TUMfooter[%
            \textbf{\tumletter@thename}\\%
            \tumletter@thestreet\ \tumletter@thestreetno\\%
            \tumletter@thezipcode\ \tumletter@thecity][%
            Tel.\ \kern.7pt\tumletter@thephone\\%
            Fax\ \tumletter@thefax\\%
            \tumletter@themail\\%
            \ifx\tumletter@theweb\@empty\relax\else%
                \tumletter@theweb\\%
            \fi
%            https://tum.de][%
%            Bayrische Landesbank\\%
%            IBAN-Nr.: DE10700500000000024866\\%
%            BIC: BYLADEMM\\%
%            Steuer-Nr.: 143/241/80037\\%
%            USt-IdNr.: DE811193231
            ]}}{}
    \else%
	    \setfoot{}{{\scriptsize\TUMfooter*[%
            \textbf{\tumletter@thename}\\%
            \tumletter@thestreet\ \tumletter@thestreetno\\%
            \tumletter@thezipcode\ \tumletter@thecity][%
            Tel.\ \kern.7pt\tumletter@thephone\\%
            Fax\ \tumletter@thefax\\%
            \tumletter@themail\\%
            \ifx\tumletter@theweb\@empty\relax\else%
                \tumletter@theweb\\%
            \fi
%            https://tum.de][%
%            Bayrische Landesbank\\%
%            IBAN-Nr.: DE10700500000000024866\\%
%            BIC: BYLADEMM\\%
%            Steuer-Nr.: 143/241/80037\\%
%            USt-IdNr.: DE811193231
            ]}}{}
    \fi%
}
\pagestyle{tumarticle}

\renewcommand\title[2][]{%
	\ifx#1\@empty\@empty
		\edef\thetitle{#2}
		\edef\@title{#2}
	\else
		\expandafter\expandafter\expandafter%
			\addto\expandafter\expandafter\csname captions#1\endcsname{%
			\def\thetitle{#2}
			\def\@title{#2}
		}
	\fi
}

\newcommand\subtitle[2][]{%
	\ifx#1\@empty\@empty
		\edef\thesubtitle{#2}
	\else
		\expandafter\expandafter\expandafter%
			\addto\expandafter\expandafter\csname captions#1\endcsname{%
			\def\thesubtitle{#2}
		}
	\fi
}

\endinput
