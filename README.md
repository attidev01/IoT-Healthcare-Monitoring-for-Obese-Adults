# An Enhanced IoT-Based Healthcare Monitoring System for Obese Adults

ICACTCE 2026 conference submission materials. An IoT healthcare monitoring
prototype for obese adults (ESP32, DS18B20, MAX30102, MQTT, ThingsBoard),
improving on the platform proposed by Gupta et al. (2020).

## Authors

- Lahcen Atti (corresponding) — atti.lahcen@gmail.com
- Mohamed Saber — mohamed.saber@edu.uiz.ac.ma
- Nacer Boubkraoui — n.boubkraoui@uiz.ac.ma
- Mouhamed Dib — m.dib@uiz.ac.ma
- Hassan Ouahi (supervisor) — h.ouahi@uiz.ac.ma

Faculty of Applied Sciences – Ait Melloul, Ibn Zohr University, Morocco.

## Submission deadline

**ICACTCE 2026 submission: August 20, 2026.** Camera-ready: September 30, 2026.
Conference: October 22–23, 2026, Marrakech.

## Directory structure

```
.
├── paper/                                          Conference paper (submission-ready)
│   ├── IoT_Healthcare_Monitoring_anonymous.tex/pdf   Double-blind review copy
│   ├── IoT_Healthcare_Monitoring_identified.tex/pdf  Camera-ready copy (real authors)
│   ├── sn-jnl.cls                                    Springer Nature LaTeX class (required by ICACTCE)
│   └── images/                                       Figures used by both papers
├── sn-article-template/                            Official Springer Nature template package (reference)
├── IoT_Module_CodeSource.ino                       ESP32 firmware source (Arduino)
├── Rapport.pdf                                     Original project report
└── Video Demo.mp4                                  Demo recording
```

## Compiling the paper

Both `.tex` files are self-contained (the required `sn-jnl.cls` is bundled
alongside them in `paper/`) and use a manual bibliography, so no `bibtex`
pass is needed — just run `pdflatex` twice to resolve cross-references:

```
cd paper
pdflatex IoT_Healthcare_Monitoring_anonymous.tex
pdflatex IoT_Healthcare_Monitoring_anonymous.tex
```

Same for the identified version. Both compile cleanly to exactly **8 pages**
(the ICACTCE 2026 limit, including references and figures), with no
overfull/underfull warnings.

## Anonymous vs. identified

ICACTCE 2026 requires double-blind review. Submit
`IoT_Healthcare_Monitoring_anonymous.pdf` (author names, affiliation, and
emails withheld; Wi-Fi credentials and the MQTT device token in the code
listings are also anonymized as placeholders, since those are real secrets
regardless of authorship). Keep `IoT_Healthcare_Monitoring_identified.pdf`
for the camera-ready version once the paper is accepted.

## Template

Per the official ICACTCE 2026 guidelines (icactceconf.com/call-for-papers),
submissions must use the **Springer Nature LaTeX template package**
(`sn-jnl.cls`, `sn-mathphys-num` numbered reference style) — not the
`llncs`/LNCS proceedings class. Both papers here are built on that template.

## Notes on paper content

The paper reports only what is documented in `Rapport.pdf` and
`IoT_Module_CodeSource.ino`: no clinical trial, statistical validation, or
IRB approval was conducted, so none is claimed. Clinical validation is
listed as future work (Sect. 7 of the paper). Photos and screenshots used
as figures are real captures from the working prototype, not renders or
mockups.
