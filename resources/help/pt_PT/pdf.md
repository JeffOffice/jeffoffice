# Ficheiros PDF e gráficas
<!-- keywords: pdf, criar pdf, guardar como pdf, exportar pdf, pdf/x, pdf/a, arquivo, arquivar, gráfica, tipografia, gráfica comercial, impressão profissional, cmyk, sangria, marcas de corte, perfil de cor, cor direta, sobreimpressão -->

Um **PDF** é um ficheiro com o mesmo aspeto em qualquer computador. Utilize-o para enviar uma publicação por correio eletrónico, publicá-la online ou enviá-la para uma gráfica.

## Criar um PDF

1. Clique em **Ficheiro > Exportar** e, em seguida, em **Criar PDF**.
2. Em **Otimizar para**, escolha uma qualidade. **dpi** significa pontos por polegada. Mais pontos dão imagens mais nítidas e ficheiros maiores.
3. Em **Páginas**, escolha **Todas as páginas**, **Página atual** ou **Páginas de** uma página **a** outra.
4. Clique em **Guardar PDF…**, escreva um nome e clique em **Guardar**.

| Opção | Imagens | Ideal para |
|---|---|---|
| Tamanho mínimo | 96 dpi | Visualização no ecrã |
| Padrão | 150 dpi | Partilha online |
| Impressão de alta qualidade | 300 dpi | Impressoras de secretária |
| Impressão comercial | Resolução total | Uma gráfica. Adiciona marcas de corte, marcas de sangria, marcas de registo, barras de cor e informações do trabalho. |

Outras opções:

- **Incluir propriedades do documento** coloca o título, o autor, o assunto e as palavras-chave no PDF.
- **Abrir o PDF depois de o guardar** mostra o PDF de imediato no seu visualizador.
- **Folhas de folheto** aparece para folhetos. Coloca duas páginas em cada lado, pela ordem de dobragem. Desmarque-a para obter uma página por cada página do PDF.

## PDF/A para guardar registos

O **PDF/A** é um PDF feito para durar anos. Contém todos os tipos de letra e utiliza cores padrão, sem efeitos de transparência. Marque **PDF/A para arquivo de longo prazo** ou clique em **Ficheiro > Exportar** e, em seguida, em **Criar PDF/A para arquivo**.

## PDF/X para uma gráfica

O **PDF/X** é um PDF mais rigoroso, pedido pelas gráficas. As cores são indicadas como quantidades de tinta, os tipos de letra estão incluídos e o corte e a sangria estão marcados. A **sangria** é a imagem ou cor adicional que ultrapassa a margem da página, para que o corte não deixe um limite branco.

1. Clique em **Ficheiro > Exportar** e, em seguida, em **Criar PDF/X para uma gráfica comercial**. Em alternativa, marque **PDF/X para uma gráfica comercial** na janela **Criar PDF**. Não pode utilizá-lo em conjunto com PDF/A.
2. Em **Condição de impressão**, escolha a que a sua gráfica indicar.
3. Se a gráfica pretender marcas de corte e sangria, escolha também **Impressão comercial** em **Otimizar para**.
4. Clique em **Guardar PDF…**.

As condições são:

- **PDF/X-1a** para offset rotativo dos EUA (SWOP) ou offset europeu (FOGRA39). Achata os efeitos de transparência.
- **PDF/X-4** para papel couché premium (FOGRA51, PSO Coated v3) ou com o perfil de cor próprio da sua gráfica (um ficheiro .icc que escolhe). Mantém os efeitos de transparência.

O perfil de cor PSO Coated v3 não está incluído no JeffPub. Na primeira vez que o utilizar, o JeffPub pede autorização antes de transferir o perfil, uma única vez, da European Color Initiative.

Antes de criar o ficheiro, o JeffPub verifica se existem imagens com menos de 150 ppi (píxeis por polegada) no tamanho impresso e tipos de letra que não estejam instalados neste computador. Clique em **Criar PDF mesmo assim**, **Abrir verificador de estrutura** ou **Cancelar**.

## Configurar as cores para uma gráfica

Clique em **Ficheiro > Propriedades** e, em seguida, no cartão **Definições de Impressão Comercial**:

- **Modelo de cor** pode ser **RGB**, **Uma só cor (direta)**, **Cores diretas**, **Cores de processo (CMYK)** ou **Cores de processo e cores diretas**. As cores de processo são as tintas ciano, magenta, amarelo e preto. Uma cor direta é uma tinta especial que a gráfica prepara. Adicione cada uma em **Cores diretas**.
- **Sobreimprimir o preto em ficheiros para gráfica (PDF/X)** faz com que o preto seja impresso por cima das cores que estão por baixo, para que uma chapa desalinhada não deixe um limite branco. Pode escolher a que se aplica: texto abaixo de um determinado tamanho, linhas ou preenchimentos.

## Tópicos relacionados

- [Imprimir uma publicação](printing)
- [Guardar como imagem, página Web ou livro eletrónico](export)
- [Verificar a estrutura](design-checker)
