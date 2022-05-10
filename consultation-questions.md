# Otázky na konzultace

[//]: # (První konzultace)

## 1. Způsob abstrakce nad cpu

### Momentálně: pseudo c#/java interface

- struct nesoucí typovou informaci (function pointery) `cpu_type_t`
- druhý co má instanci tohoto typu a pointer na data   `general_cpu_t`

typové funkce mají dané parametry, první je pointer na instanci.

#### problém

- typ ukazatele v definici function pointerů v `cpu_type_t` musí být slabě typovaný
- samotné funkce bych chtěl mít silně typované (aby se daly bezpečně volat i odjinaď)
- přiřazuju ukazatel na silně typovanou funkci do slabě typovaného function pointeru
- dělat interface funkce, co jen castujou pointer mi připadá jako bloat

### Alternativa

zavést enum a virtuální dispatch dělat přes switch

- hůř rozšiřitelné

### Poznámky

- z velké části programu se přistupuje k cpu jen jednou a přes jeho číselné id

### Odpověď

Buď castnout function ptr (preferováno) nebo napsat wrappery (upožňuje navíc debug výpisy a asserty)

## 2. ukládání CSR

- v risc-v je 12-bit address space CSR
- ne všechny jsou využity (reálně tak ~280 použito)
- přistupuje se k nim přes jejich adresu
- je lepší mít to alokované celé (16 kiB), nebo to mít řídce a pak mít switch?

### odpověď

- asi bude zapotřebí ten switch
- r/w se dá dělat za decodu
- pak může vést na specializované, předdekódované specializace instrukcí csrrd csrwr
- začít bude lepší superintrukcí bez předdekódování
- zatím odložit, lepší je začít non-privileged instrukcemi

## 3. Debug

- hodně zadrátovaný mips
- můžu nechat na později?
  - jo

## 4. `mtime` a `mtimecmp`

- v specifikaci `risc-v` mají být memory mapped registry
- přístupné jen v machine-mode
- nikde se neříká, jak se s nima má manipulovat

### odpověď

- pro msim to znamená, že tam bude device.
- radši mám naimplementovat nový rv timer, než používat `dtime`
- při implementaci `RDTIME` si vytáhnu předdefinovaný device (z configu, ať má dobrou adresu), nebo založím nový

[//]: # (Druhé konzultace)

## 5. Otázky k mojí implementaci

- dispatch v `mnemonics_decode`
- pojmenovávání funkcí pro instrukci
- copy-paste instrukce
  - má smysl vymýšlet něco lepšího a refactorovat?

## 6. MIPS ll/sc

- kolize s kratšími zápisy na nezarovnané adresy

```assembly
ll t0, 0x04(0)
sh t1, 0x06(0)
sc t0, 0x04(0)
```

## 7. MSIM bug

- `set help = xxx` nefunguje
  - při vykonávání `help` se nepřeskočí `=`
  - u sebe jsem to spravil
- test s `bad_status_ksu` se donekonečna zacyklí v exception handleru
  - handler je mimo paměť, tedy instruction fetch vyhodí výjimku, která způsobí, že procesor skočí na exception handler...

## 8. Kam dál

### exception/interrupt handling

- instrukce co vyhazuje výjimku nastaví potřebná data v CSR?
  - chybná instrukce => vyplní obecný handler?
  - chybná adresa (target) => vyplní instrukce?
  - adresa samotné instrukce => vyplní obecný handler?
- samotný jump na handler už bokem?

### překlad adres

- až po exception handlingu

### podpora všech CSR

- přesněji podpora instrukcí co s nimi interagují
- průběžně s předchozími, dle toho co bude potřeba
- zbytek lze dodělat až posléze
- (jaké jsou dobré hw eventy na profiling)
  - (inspirace by se dala vykrást z vtune)
