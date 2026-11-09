# Ficheiros PDF e gráficas

<!-- keywords: pdf, criar pdf, guardar como pdf, exportar pdf, pdf/x, pdf/a, arquivo, gráfica, impressão comercial, cmyk, sangria, marcas de corte, perfil de cor, cor direta, sobreimpressão, palavra-passe, encriptar, proteger com palavra-passe -->

Um **PDF** é um ficheiro que tem o mesmo aspeto em qualquer computador. Use-o para enviar por correio eletrónico uma publicação, publicá-la online ou enviá-la para uma gráfica.

## Criar um PDF

1. Clique em **Ficheiro > Exportar** e, a seguir, em **Criar PDF**.
2. Em **Otimizar para**, escolha uma qualidade. **ppp** significa pontos por polegada. Mais pontos dão imagens mais nítidas e ficheiros maiores.
3. Em **Páginas**, escolha **Todas as páginas**, **Página atual** ou **Páginas de** uma página **a** outra.
4. Clique em **Guardar PDF…**, escreva um nome e clique em **Guardar**.

| Opção | Imagens | Indicado para |
|---|---|---|
| Tamanho mínimo | 96 ppp | Visualização no ecrã |
| Padrão | 150 ppp | Partilha online |
| Impressão de alta qualidade | 300 ppp | Impressoras de secretária |
| Impressão comercial | Resolução total | Uma gráfica. Acrescenta marcas de corte, marcas de sangria, marcas de registo, barras de cor e informações do trabalho. |

Outras opções:

- **Incluir propriedades do documento** coloca o título, o autor, o assunto e as palavras-chave no PDF.
- **Abrir o PDF depois de o guardar** mostra o PDF no seu visualizador de imediato.
- **Folhas do folheto** aparece nos folhetos. Coloca duas páginas em cada lado, pela ordem de dobragem. Desmarque para ter uma página por página do PDF.

## PDF/A para conservar registos

**PDF/A** é um PDF criado para durar anos. Inclui todos os tipos de letra e usa cores padrão, sem efeitos de transparência. Marque **PDF/A para arquivo de longo prazo** ou clique em **Ficheiro > Exportar** e, a seguir, em **Criar PDF/A para Arquivo**.

## PDF/X para uma gráfica

**PDF/X** é um PDF mais rigoroso, que as gráficas costumam pedir. As cores são indicadas como quantidades de tinta, os tipos de letra ficam incluídos e o corte e a sangria ficam assinalados. A **sangria** é imagem ou cor extra que ultrapassa a margem da página, para que o corte não deixe nenhuma borda branca.

1. Clique em **Ficheiro > Exportar** e, a seguir, em **Criar PDF/X para uma Gráfica Comercial**. Ou marque **PDF/X para uma gráfica comercial** na janela **Criar PDF**. Não pode ser usado com PDF/A.
2. Em **Condição de impressão**, escolha a que a sua gráfica indicar.
3. Se a gráfica pedir marcas de corte e sangria, escolha também **Impressão comercial** em **Otimizar para**.
4. Clique em **Guardar PDF…**.

As condições são:

- **PDF/X-1a** para offset rotativo dos EUA (SWOP) ou offset europeu (FOGRA39). Achata os efeitos de transparência.
- **PDF/X-4** para papel couché premium (FOGRA51, PSO Coated v3), ou com o perfil de cor da sua impressora (um ficheiro .icc que escolhe). Mantém os efeitos de transparência.

O perfil de cor PSO Coated v3 não vem com o JeffPub. Na primeira vez que o usar, o JeffPub pergunta antes de transferir o perfil, uma única vez, da European Color Initiative.

Antes de criar o ficheiro, o JeffPub verifica se há imagens abaixo de 150 ppi (pixels por polegada) no tamanho em que são impressas e se há tipos de letra em falta neste computador. Clique em **Criar PDF Mesmo Assim**, **Abrir verificador de estrutura** ou **Cancelar**.

## Proteger um PDF com palavra-passe

Na janela **Criar PDF**, clique em **Encriptar com uma Palavra-passe…**. Escreva a palavra-passe duas vezes. Quem abrir o PDF terá de a escrever. A palavra-passe protege mesmo o ficheiro: as páginas são cifradas com encriptação forte (AES-256) e ninguém as consegue ler sem ela.

Também pode marcar **Restringir impressão, cópia e edição**, indicar uma segunda palavra-passe e escolher o que é permitido. Estas limitações são apenas um pedido. A maioria dos leitores de PDF respeita-as, mas não impedem quem queira contorná-las; por isso, não confie nelas para manter um ficheiro privado.

Uma palavra-passe esquecida não pode ser recuperada, por isso guarde-a num local seguro. A palavra-passe é usada apenas nesse PDF e nunca é guardada. Um PDF com palavra-passe não pode ser PDF/A nem PDF/X, pelo que estas duas opções ficam desativadas enquanto houver uma palavra-passe definida. Em **Ficheiro > Partilhar**, **Encriptar com Palavra-passe…** aparece acima de **Enviar como PDF** e coloca uma palavra-passe no PDF enviado por correio eletrónico.

Quando coloca um PDF com palavra-passe como imagem, o JeffPub pede a palavra-passe. Se estiver errada, o JeffPub volta a pedir; clique em **Cancelar** para parar. A página na sua publicação é uma cópia e não precisa de palavra-passe. Se o proprietário do PDF pedir que não seja copiado, o JeffPub coloca-o na mesma e mostra uma nota a dizê-lo; esse pedido não é um bloqueio.

Uma imagem ligada a um PDF com palavra-passe aparece como **Não atualizada** quando abre a publicação, e o JeffPub nunca pede a palavra-passe por iniciativa própria. Clique em **Atualizar Ligação** para que a palavra-passe seja pedida.

## Definir cores para uma gráfica

Clique em **Ficheiro > Informações** e, a seguir, no cartão **Definições de Impressão Comercial**:

- **Modelo de cor** é **RGB**, **Uma só cor (direta)**, **Cores diretas**, **Cores de processo (CMYK)** ou **Cores de processo e cores diretas**. As cores de processo são as tintas ciano, magenta, amarelo e preto. Uma cor direta é uma tinta especial que a gráfica prepara. Adicione cada uma em **Cores diretas**.
- **Sobreimprimir o preto em ficheiros para gráfica (PDF/X)** faz com que o preto seja impresso por cima das cores que estão por baixo, para que um desalinhamento da chapa não deixe nenhuma borda branca. Escolhe o que é abrangido: texto abaixo de um certo tamanho, linhas ou preenchimentos.

## Ver também

- [Imprimir uma publicação](printing)
- [Guardar como imagem, página Web ou livro eletrónico](export)
- [Verificar a estrutura](design-checker)
