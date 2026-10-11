# Arquivos PDF e gráficas
<!-- keywords: pdf, criar pdf, salvar como pdf, exportar pdf, pdf/x, pdf/a, arquivamento, arquivar, gráfica, gráfica comercial, impressão comercial, cmyk, sangria, marcas de corte, perfil de cores, cor especial, sobreimpressão -->

Um **PDF** é um arquivo com a mesma aparência em qualquer computador. Use um PDF para enviar uma publicação por email, publicá-la online ou enviá-la a uma gráfica.

## Criar um PDF

1. Clique em **Arquivo > Exportar** e em **Criar PDF**.
2. Em **Otimizar para**, escolha uma qualidade. **dpi** significa pontos por polegada. Mais pontos geram imagens mais nítidas e arquivos maiores.
3. Em **Páginas**, escolha **Todas as páginas**, **Página atual** ou **Páginas de** uma página **a** outra.
4. Clique em **Salvar PDF…**, digite um nome e clique em **Salvar**.

| Opção | Imagens | Ideal para |
|---|---|---|
| Tamanho mínimo | 96 dpi | Exibição na tela |
| Padrão | 150 dpi | Compartilhamento online |
| Impressão de alta qualidade | 300 dpi | Impressoras de mesa |
| Gráfica comercial | Resolução total | Uma gráfica. Adiciona marcas de corte, marcas de sangria, marcas de registro, barras de cores e informações do trabalho. |

Outras opções:

- **Incluir propriedades do documento** coloca o título, o autor, o assunto e as palavras-chave no PDF.
- **Abrir o PDF depois de salvá-lo** mostra o PDF no seu visualizador imediatamente.
- **Folhas de livreto** aparece para livretos. Essa opção coloca duas páginas em cada lado, na ordem de dobra. Desmarque-a para ter uma página por página do PDF.

## PDF/A para guardar registros

O **PDF/A** é um PDF feito para durar muitos anos. Ele contém todas as fontes e usa cores padrão, sem efeitos de transparência. Marque **PDF/A para arquivamento de longo prazo** ou clique em **Arquivo > Exportar** e em **Criar PDF/A para arquivamento**.

## PDF/X para uma gráfica

O **PDF/X** é um PDF mais rigoroso, exigido pelas gráficas. As cores são definidas como quantidades de tinta, as fontes ficam dentro do arquivo e o refile e a sangria são marcados. A **sangria** é uma área extra de imagem ou cor que ultrapassa a borda da página, para que o refile não deixe borda branca.

1. Clique em **Arquivo > Exportar** e em **Criar PDF/X para uma gráfica**. Ou marque **PDF/X para uma gráfica comercial** na janela **Criar PDF**. Não é possível usá-lo junto com o PDF/A.
2. Em **Condição de impressão**, escolha a condição indicada pela sua gráfica.
3. Se a gráfica quiser marcas de corte e sangria, escolha também **Gráfica comercial** em **Otimizar para**.
4. Clique em **Salvar PDF…**.

As condições são:

- **PDF/X-1a** para offset rotativa dos EUA (SWOP) ou offset europeu (FOGRA39). Nivela os efeitos de transparência.
- **PDF/X-4** para papel revestido premium (FOGRA51, PSO Coated v3) ou com o perfil de cores da sua gráfica (um arquivo .icc que você escolhe). Mantém os efeitos de transparência.

O perfil de cores PSO Coated v3 não vem com o JeffPub. Na primeira vez que você o usar, o JeffPub pedirá permissão antes de baixar o perfil, uma única vez, da European Color Initiative.

Antes de criar o arquivo, o JeffPub verifica se há imagens com menos de 150 ppi (pixels por polegada) no tamanho impresso e fontes ausentes neste computador. Clique em **Criar PDF mesmo assim**, **Abrir verificador de design** ou **Cancelar**.

## Configurar cores para uma gráfica

Clique em **Arquivo > Propriedades** e no cartão **Configurações de impressão comercial**:

- **Modelo de cores** pode ser **RGB**, **Cor única (especial)**, **Cores especiais**, **Cores de processo (CMYK)** ou **Cores de processo e especiais**. As cores de processo são as tintas ciano, magenta, amarelo e preto. Uma cor especial é uma tinta especial preparada pela gráfica. Adicione cada uma em **Cores especiais**.
- **Sobreimpressão de preto em arquivos para gráfica (PDF/X)** faz o preto ser impresso sobre as cores abaixo dele, para que uma chapa desalinhada não deixe borda branca. Você escolhe o que ela abrange: texto abaixo de um tamanho, linhas ou preenchimentos.

## Relacionados

- [Imprimir uma publicação](printing)
- [Salvar como imagem, página da Web ou e-book](export)
- [Verificar o design](design-checker)
