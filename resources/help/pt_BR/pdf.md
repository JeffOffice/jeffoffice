# Arquivos PDF e gráficas

<!-- keywords: pdf, criar pdf, salvar como pdf, exportar pdf, pdf/x, pdf/a, arquivamento, gráfica, gráfica comercial, cmyk, sangria, marcas de corte, perfil de cor, cor especial, sobreimpressão, senha, criptografar, proteger com senha -->

Um **PDF** é um arquivo que tem a mesma aparência em qualquer computador. Use-o para enviar uma publicação por email, publicá-la na internet ou mandá-la para uma gráfica.

## Criar um PDF

1. Clique em **Arquivo > Exportar** e depois em **Criar PDF**.
2. Em **Otimizar para**, escolha uma qualidade. **dpi** significa pontos por polegada. Mais pontos deixam as imagens mais nítidas e os arquivos maiores.
3. Em **Páginas**, escolha **Todas as páginas**, **Página atual** ou **Páginas de** uma página **até** outra.
4. Clique em **Salvar PDF…**, digite um nome e clique em **Salvar**.

| Opção | Imagens | Ideal para |
|---|---|---|
| Tamanho mínimo | 96 dpi | Visualização na tela |
| Padrão | 150 dpi | Compartilhamento na internet |
| Impressão de alta qualidade | 300 dpi | Impressoras de mesa |
| Gráfica comercial | Resolução total | Uma gráfica. Adiciona marcas de corte, marcas de sangria, marcas de registro, barras de cores e informações do trabalho. |

Outras opções:

- **Incluir propriedades do documento** coloca o título, o autor, o assunto e as palavras-chave no PDF.
- **Abrir o PDF depois de salvá-lo** mostra o PDF no seu leitor logo em seguida.
- **Folhas do livreto** aparece para livretos. Ela coloca duas páginas em cada lado, na ordem de dobra. Desmarque para ter uma página por página do PDF.

## PDF/A para guardar registros

**PDF/A** é um PDF feito para durar anos. Ele contém todas as fontes e usa cores padronizadas, sem efeitos de transparência. Marque **PDF/A para arquivamento de longo prazo** ou clique em **Arquivo > Exportar** e depois em **Criar PDF/A para arquivamento**.

## PDF/X para uma gráfica

**PDF/X** é um PDF mais rigoroso, que as gráficas costumam pedir. As cores são dadas como quantidades de tinta, as fontes ficam incorporadas e o corte e a sangria são marcados. A **sangria** é a imagem ou cor extra que ultrapassa a borda da página, para que o corte não deixe uma borda branca.

1. Clique em **Arquivo > Exportar** e depois em **Criar PDF/X para uma gráfica**. Ou marque **PDF/X para uma gráfica** na janela **Criar PDF**. Não é possível usá-lo junto com PDF/A.
2. Em **Condição de impressão**, escolha a que a sua gráfica indicar.
3. Se a gráfica pedir marcas de corte e sangria, escolha também **Gráfica comercial** em **Otimizar para**.
4. Clique em **Salvar PDF…**.

As condições são:

- **PDF/X-1a** para offset rotativa dos EUA (SWOP) ou offset europeu (FOGRA39). Ele achata os efeitos de transparência.
- **PDF/X-4** para papel revestido premium (FOGRA51, PSO Coated v3) ou com o perfil de cor da sua própria impressora (um arquivo .icc que você escolhe). Ele mantém os efeitos de transparência.

O perfil de cor PSO Coated v3 não vem com o JeffPub. Na primeira vez que você o usar, o JeffPub pede sua confirmação antes de baixá-lo, uma única vez, da European Color Initiative.

Antes de criar o arquivo, o JeffPub verifica se há imagens abaixo de 150 ppi (pixels por polegada) em seu tamanho impresso e se há fontes ausentes neste computador. Clique em **Criar PDF mesmo assim**, **Abrir verificador de design** ou **Cancelar**.

## Proteger um PDF com senha

Na janela **Criar PDF**, clique em **Criptografar com senha…**. Digite a senha duas vezes. Quem abrir o PDF terá de digitá-la. A senha protege o arquivo de fato: as páginas são embaralhadas com criptografia forte (AES-256), e ninguém consegue lê-las sem ela.

Você também pode marcar **Restringir impressão, cópia e edição**, informar uma segunda senha e escolher o que é permitido. Essas restrições são apenas um pedido. A maioria dos leitores de PDF as respeita, mas elas não impedem quem queira contorná-las, por isso não conte com elas para manter um arquivo em sigilo.

Uma senha esquecida não pode ser recuperada, por isso guarde-a em local seguro. A senha vale apenas para esse PDF e nunca é salva. Um PDF com senha não pode ser PDF/A nem PDF/X, por isso essas duas opções ficam desativadas enquanto houver uma senha definida. Em **Arquivo > Compartilhar**, **Criptografar com senha…** acima de **Enviar como PDF** coloca uma senha no PDF de um email.

Ao inserir como imagem um PDF que tem senha, o JeffPub pede a senha. Se ela estiver errada, o JeffPub pede novamente; clique em **Cancelar** para parar. A página na sua publicação é uma cópia e não precisa de senha. Se o dono do PDF pedir que ele não seja copiado, o JeffPub insere o arquivo mesmo assim e mostra um aviso sobre isso; esse pedido não é um bloqueio.

Uma imagem vinculada a um PDF com senha aparece como **Não atualizada** quando você abre a publicação, e o JeffPub nunca pede a senha por conta própria. Clique em **Atualizar vínculo** para ser solicitado.

## Configurar as cores para uma gráfica

Clique em **Arquivo > Informações** e depois no cartão **Configurações de Impressão Comercial**:

- **Modelo de cores** pode ser **RGB**, **Cor única (especial)**, **Cores especiais**, **Cores de processo (CMYK)** ou **Cores de processo e especiais**. As cores de processo são as tintas ciano, magenta, amarela e preta. Uma cor especial é uma tinta própria que a gráfica prepara. Adicione cada uma em **Cores especiais**.
- **Sobreimpressão de preto em arquivos para gráfica (PDF/X)** faz o preto imprimir sobre as cores abaixo dele, para que uma chapa deslocada não deixe uma borda branca. Você escolhe o que é coberto: textos abaixo de um tamanho, linhas ou preenchimentos.

## Veja também

- [Imprimir uma publicação](printing)
- [Salvar como imagem, página da Web ou e-book](export)
- [Verificar seu design](design-checker)
